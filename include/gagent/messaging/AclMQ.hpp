/*
 * AclMQ.hpp — Transport ZeroMQ pour la messagerie FIPA ACL
 *
 * Chaque agent dispose d'un socket PULL dédié.
 * Transport local  : ipc:///tmp/acl_<nom>
 * Transport réseau : variable d'environnement GAGENT_ENDPOINT_<NOM>=tcp://host:port
 *
 * Usage (API inchangée) :
 *   #include <gagent/messaging/AclMQ.hpp>
 *   using namespace gagent::messaging;
 *
 *   acl_send("bob", msg);
 *   auto opt = acl_receive("alice", 5000);
 */

#ifndef GAGENT_ACLMQ_HPP_
#define GAGENT_ACLMQ_HPP_

#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/utils/Logger.hpp>
#include <zmq.h>
#include <optional>
#include <string>
#include <map>
#include <mutex>
#include <cstdlib>
#include <cctype>
#include <unistd.h>

namespace gagent {
namespace messaging {

// ── Endpoint ──────────────────────────────────────────────────────────────────

/**
 * Retourne l'endpoint ZMQ pour un agent donné.
 *
 * Priorité :
 *   1. Variable d'environnement GAGENT_ENDPOINT_<NOM> (ex: tcp://192.168.1.10:5555)
 *   2. IPC local : ipc:///tmp/acl_<nom>
 */
inline std::string acl_endpoint(const std::string& name)
{
    // Construire le nom de la variable : GAGENT_ENDPOINT_<NOM_EN_MAJUSCULES>
    std::string var = "GAGENT_ENDPOINT_";
    for (char c : name) var += static_cast<char>(std::toupper(c));
    const char* ep = std::getenv(var.c_str());
    if (ep && ep[0] != '\0') return ep;
    return "ipc:///tmp/acl_" + name;
}

// ── Contexte ZMQ (un par processus, lazy init) ────────────────────────────────

inline void* zmq_ctx()
{
    static void*  ctx     = nullptr;
    static pid_t  ctx_pid = 0;

    pid_t cur = ::getpid();
    if (ctx_pid != cur) {
        // Première utilisation dans ce processus (ou après fork)
        if (ctx) zmq_ctx_destroy(ctx);
        ctx     = zmq_ctx_new();
        ctx_pid = cur;
    }
    return ctx;
}

// ── Cache des sockets PULL (un par nom d'agent par processus) ─────────────────

class PullCache {
public:
    static PullCache& instance() {
        static PullCache inst;
        return inst;
    }

    /**
     * Retourne le socket PULL pour cet agent, en le créant et le bindant
     * à son endpoint si c'est la première fois.
     */
    void* get(const std::string& name) {
        std::lock_guard<std::mutex> lock(mtx_);

        // Invalider le cache si on est dans un nouveau processus (après fork)
        pid_t cur = ::getpid();
        if (pid_ != cur) {
            for (auto& [n, s] : sockets_) zmq_close(s);
            sockets_.clear();
            pid_ = cur;
        }

        auto it = sockets_.find(name);
        if (it != sockets_.end()) return it->second;

        void* sock = zmq_socket(zmq_ctx(), ZMQ_PULL);
        if (!sock) return nullptr;

        int linger = 0;
        zmq_setsockopt(sock, ZMQ_LINGER, &linger, sizeof(linger));

        std::string ep = acl_endpoint(name);

        // Supprimer le fichier socket IPC résiduel d'un run précédent
        if (ep.rfind("ipc://", 0) == 0) {
            ::unlink(ep.substr(6).c_str());
        }

        if (zmq_bind(sock, ep.c_str()) != 0) {
            zmq_close(sock);
            return nullptr;
        }

        sockets_[name] = sock;
        return sock;
    }

    /** Ferme le socket et nettoie le fichier IPC. */
    void remove(const std::string& name) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = sockets_.find(name);
        if (it == sockets_.end()) return;
        zmq_close(it->second);
        sockets_.erase(it);

        // Supprimer le fichier socket IPC si c'est un endpoint local
        std::string ep = acl_endpoint(name);
        if (ep.rfind("ipc://", 0) == 0) {
            std::string path = ep.substr(6); // retirer "ipc://"
            ::unlink(path.c_str());
        }
    }

private:
    PullCache() : pid_(::getpid()) {}

    std::mutex mtx_;
    std::map<std::string, void*> sockets_;
    pid_t pid_;
};

// ── Cache des sockets PUSH (un par destination par processus) ─────────────────
//
// Évite le "slow joiner" : la connexion est établie une seule fois et réutilisée.

class PushCache {
public:
    static PushCache& instance() {
        static PushCache inst;
        return inst;
    }

    /**
     * Envoie <data> vers <to> de façon thread-safe.
     *
     * Le mutex est tenu pendant toute l'opération (connexion + zmq_send)
     * afin qu'un seul thread à la fois utilise chaque socket ZMQ
     * (les sockets ZMQ ne sont pas thread-safe).
     *
     * @return true si l'envoi a réussi
     */
    bool send(const std::string& to, const char* data, size_t len) {
        std::lock_guard<std::mutex> lock(mtx_);

        void* sock = getOrCreate(to);
        if (!sock) return false;

        return zmq_send(sock, data, len, 0) >= 0;
    }

    /** Ferme tous les sockets PUSH en respectant le linger.
     *  À appeler avant _exit() pour garantir la livraison des derniers messages. */
    void flush() {
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto& [n, s] : sockets_) zmq_close(s);
        sockets_.clear();
    }

private:
    PushCache() : pid_(::getpid()) {}

    /** Doit être appelé avec mtx_ déjà verrouillé. */
    void* getOrCreate(const std::string& to) {
        pid_t cur = ::getpid();
        if (pid_ != cur) {
            for (auto& [n, s] : sockets_) zmq_close(s);
            sockets_.clear();
            pid_ = cur;
        }

        auto it = sockets_.find(to);
        if (it != sockets_.end()) return it->second;

        void* sock = zmq_socket(zmq_ctx(), ZMQ_PUSH);
        if (!sock) return nullptr;

        int linger = 2000;
        zmq_setsockopt(sock, ZMQ_LINGER, &linger, sizeof(linger));

        int sndtmo = 2000;
        zmq_setsockopt(sock, ZMQ_SNDTIMEO, &sndtmo, sizeof(sndtmo));

        std::string ep = acl_endpoint(to);
        if (zmq_connect(sock, ep.c_str()) != 0) {
            zmq_close(sock);
            return nullptr;
        }

        // Slow-joiner workaround : laisser la connexion IPC s'établir
        // avant le premier envoi (ZMQ connecte en arrière-plan).
        usleep(1000); // 1 ms

        sockets_[to] = sock;
        return sock;
    }

    std::mutex mtx_;
    std::map<std::string, void*> sockets_;
    pid_t pid_;
};

// ── API publique ──────────────────────────────────────────────────────────────

/**
 * Pré-bind le socket PULL pour <name> sans attendre de message.
 * À appeler dans onStart() pour garantir que le socket est prêt
 * avant d'envoyer un premier message qui pourrait générer une réponse.
 */
inline void acl_bind(const std::string& name)
{
    PullCache::instance().get(name);
}

/**
 * Envoie un ACLMessage à l'agent <to>.
 * Utilise un socket PUSH persistant (PushCache) pour éviter le "slow joiner".
 * @return true si l'envoi a réussi
 */
inline bool acl_send(const std::string& to, const ACLMessage& msg)
{
    std::string raw = msg.toString();

    bool ok = PushCache::instance().send(to, raw.c_str(), raw.size());
    if (ok) {
        std::string recv_name = msg.getReceivers().empty()
                              ? to : msg.getReceivers()[0].name;
        LOG_JSON("acl_send",
            {"from",    msg.getSender().name},
            {"to",      recv_name},
            {"perf",    ACLMessage::performativeToString(msg.getPerformative())},
            {"conv",    msg.getConversationId()},
            {"content", msg.getContent().substr(0, 120)}
        );
    }
    return ok;
}

/**
 * Reçoit un ACLMessage depuis la queue de <my_name>.
 * Bloquant jusqu'à timeout_ms.
 * @return nullopt si timeout ou erreur
 */
inline std::optional<ACLMessage> acl_receive(const std::string& my_name,
                                              int timeout_ms = 5000)
{
    void* sock = PullCache::instance().get(my_name);
    if (!sock) return std::nullopt;

    zmq_pollitem_t items[] = { { sock, 0, ZMQ_POLLIN, 0 } };
    int rc = zmq_poll(items, 1, timeout_ms);
    if (rc <= 0) return std::nullopt;

    zmq_msg_t zmsg;
    zmq_msg_init(&zmsg);
    int n = zmq_msg_recv(&zmsg, sock, 0);
    if (n < 0) {
        zmq_msg_close(&zmsg);
        return std::nullopt;
    }

    std::string raw(static_cast<char*>(zmq_msg_data(&zmsg)),
                    zmq_msg_size(&zmsg));
    zmq_msg_close(&zmsg);

    auto result = ACLMessage::parse(raw);
    if (result) {
        LOG_JSON("acl_recv",
            {"to",      my_name},
            {"from",    result->getSender().name},
            {"perf",    ACLMessage::performativeToString(result->getPerformative())},
            {"conv",    result->getConversationId()},
            {"content", result->getContent().substr(0, 120)}
        );
    }
    return result;
}

/**
 * Libère le socket de <name> et supprime le fichier IPC.
 * À appeler dans takeDown() de l'agent.
 */
inline void acl_unlink(const std::string& name)
{
    PullCache::instance().remove(name);
}

/**
 * Ferme tous les sockets PUSH en respectant le linger ZMQ.
 * À appeler juste avant _exit() pour garantir la livraison des messages en attente.
 * Sans cela, _exit() ferme les descripteurs brutalement, court-circuitant le linger.
 */
inline void acl_flush()
{
    PushCache::instance().flush();
}

} // namespace messaging
} // namespace gagent

#endif /* GAGENT_ACLMQ_HPP_ */
