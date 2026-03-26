/*
 * AclMQ.hpp — Transport ZeroMQ pour la messagerie FIPA ACL
 *
 * Mode local  : ipc:///tmp/acl_<nom>
 * Mode cluster: tcp://<slave_ip>:<port> (port dérivé du nom de l'agent)
 *
 * Usage :
 *   #include <gagent/messaging/AclMQ.hpp>
 *   using namespace gagent::messaging;
 *
 *   std::string ep = acl_bind("alice");   // retourne l'endpoint à passer à l'AMS
 *   acl_send("bob", msg);
 *   auto opt = acl_receive("alice", 5000);
 */

#ifndef GAGENT_ACLMQ_HPP_
#define GAGENT_ACLMQ_HPP_

#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/platform/PlatformConfig.hpp>
#include <gagent/platform/AMSClient.hpp>
#include <gagent/utils/Logger.hpp>
#include <zmq.h>
#include <optional>
#include <string>
#include <map>
#include <mutex>
#include <functional>
#include <cstdlib>
#include <cctype>
#include <unistd.h>

namespace gagent {
namespace messaging {

// ── Endpoint local (IPC) ──────────────────────────────────────────────────────

/**
 * Retourne l'endpoint IPC local pour un agent.
 * En mode cluster, l'endpoint réel est négocié par acl_bind().
 *
 * Priorité :
 *   1. Variable d'environnement GAGENT_ENDPOINT_<NOM>
 *   2. IPC local : ipc:///tmp/acl_<nom>
 */
inline std::string acl_endpoint_ipc(const std::string& name)
{
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
        if (ctx) zmq_ctx_destroy(ctx);
        ctx     = zmq_ctx_new();
        ctx_pid = cur;
    }
    return ctx;
}

// ── Cache des sockets PULL ────────────────────────────────────────────────────

class PullCache {
public:
    static PullCache& instance() {
        static PullCache inst;
        return inst;
    }

    /**
     * Retourne le socket PULL pour cet agent (crée + bind si 1ère fois).
     * @param[out] out_endpoint  Endpoint public (pour enregistrement AMS)
     */
    void* get(const std::string& name, std::string* out_endpoint = nullptr) {
        std::lock_guard<std::mutex> lock(mtx_);

        pid_t cur = ::getpid();
        if (pid_ != cur) {
            for (auto& [n, s] : sockets_) zmq_close(s);
            sockets_.clear();
            endpoints_.clear();
            pid_ = cur;
        }

        auto it = sockets_.find(name);
        if (it != sockets_.end()) {
            if (out_endpoint) *out_endpoint = endpoints_[name];
            return it->second;
        }

        void* sock = zmq_socket(zmq_ctx(), ZMQ_PULL);
        if (!sock) return nullptr;

        int linger = 0;
        zmq_setsockopt(sock, ZMQ_LINGER, &linger, sizeof(linger));

        auto& cfg = gagent::platform::PlatformConfig::instance();
        std::string ep;

        if (cfg.isCluster()) {
            // Mode cluster : bind TCP
            int base_port = cfg.basePort();
            uint16_t h    = static_cast<uint16_t>(
                                std::hash<std::string>{}(name) % 15000);
            bool bound = false;
            for (int attempt = 0; attempt < 100; ++attempt) {
                uint16_t port = static_cast<uint16_t>(base_port) + h
                                + static_cast<uint16_t>(attempt);
                std::string bind_ep = "tcp://*:" + std::to_string(port);
                if (zmq_bind(sock, bind_ep.c_str()) == 0) {
                    ep    = "tcp://" + cfg.slaveIP() + ":"
                          + std::to_string(port);
                    bound = true;
                    break;
                }
            }
            if (!bound) {
                zmq_close(sock);
                return nullptr;
            }
        } else {
            // Mode local : bind IPC
            ep = acl_endpoint_ipc(name);
            if (ep.rfind("ipc://", 0) == 0)
                ::unlink(ep.substr(6).c_str());
            if (zmq_bind(sock, ep.c_str()) != 0) {
                zmq_close(sock);
                return nullptr;
            }
        }

        sockets_[name]   = sock;
        endpoints_[name] = ep;
        if (out_endpoint) *out_endpoint = ep;
        return sock;
    }

    /** Ferme le socket et nettoie le fichier IPC si applicable. */
    void remove(const std::string& name) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = sockets_.find(name);
        if (it == sockets_.end()) return;
        zmq_close(it->second);
        sockets_.erase(it);

        auto eit = endpoints_.find(name);
        if (eit != endpoints_.end()) {
            if (eit->second.rfind("ipc://", 0) == 0)
                ::unlink(eit->second.substr(6).c_str());
            endpoints_.erase(eit);
        }
    }

private:
    PullCache() : pid_(::getpid()) {}

    std::mutex mtx_;
    std::map<std::string, void*>       sockets_;
    std::map<std::string, std::string> endpoints_;
    pid_t pid_;
};

// ── Cache des sockets PUSH ────────────────────────────────────────────────────

class PushCache {
public:
    static PushCache& instance() {
        static PushCache inst;
        return inst;
    }

    bool send(const std::string& to, const char* data, size_t len) {
        std::lock_guard<std::mutex> lock(mtx_);
        void* sock = getOrCreate(to);
        if (!sock) return false;
        return zmq_send(sock, data, len, 0) >= 0;
    }

    void flush() {
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto& [n, s] : sockets_) zmq_close(s);
        sockets_.clear();
    }

private:
    PushCache() : pid_(::getpid()) {}

    void* getOrCreate(const std::string& to) {
        pid_t cur = ::getpid();
        if (pid_ != cur) {
            for (auto& [n, s] : sockets_) zmq_close(s);
            sockets_.clear();
            pid_ = cur;
        }

        auto it = sockets_.find(to);
        if (it != sockets_.end()) return it->second;

        // Résoudre l'endpoint
        std::string ep;
        auto& cfg = gagent::platform::PlatformConfig::instance();

        if (cfg.isCluster()) {
            // Interroger l'AMS pour obtenir l'endpoint de l'agent cible
            gagent::platform::AMSClient ams;
            auto info = ams.lookup(to);
            if (info && !info->address.empty())
                ep = info->address;
            else
                ep = acl_endpoint_ipc(to);   // fallback IPC (agent local)
        } else {
            ep = acl_endpoint_ipc(to);
        }

        void* sock = zmq_socket(zmq_ctx(), ZMQ_PUSH);
        if (!sock) return nullptr;

        int linger = 2000;
        zmq_setsockopt(sock, ZMQ_LINGER, &linger, sizeof(linger));

        int sndtmo = 2000;
        zmq_setsockopt(sock, ZMQ_SNDTIMEO, &sndtmo, sizeof(sndtmo));

        if (zmq_connect(sock, ep.c_str()) != 0) {
            zmq_close(sock);
            return nullptr;
        }

        // Slow-joiner : laisser la connexion s'établir
        usleep(1000);

        sockets_[to] = sock;
        return sock;
    }

    std::mutex mtx_;
    std::map<std::string, void*> sockets_;
    pid_t pid_;
};

// ── API publique ──────────────────────────────────────────────────────────────

/**
 * Pré-bind le socket PULL pour <name>.
 * @return L'endpoint public à passer à AMSClient.registerAgent().
 *         En mode local : "ipc:///tmp/acl_<name>"
 *         En mode cluster : "tcp://<slave_ip>:<port>"
 */
inline std::string acl_bind(const std::string& name)
{
    std::string ep;
    PullCache::instance().get(name, &ep);
    return ep;
}

/**
 * Envoie un ACLMessage à l'agent <to>.
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
 * Libère le socket de <name> et supprime le fichier IPC si applicable.
 */
inline void acl_unlink(const std::string& name)
{
    PullCache::instance().remove(name);
}

/**
 * Ferme tous les sockets PUSH en respectant le linger ZMQ.
 */
inline void acl_flush()
{
    PushCache::instance().flush();
}

} // namespace messaging
} // namespace gagent

#endif /* GAGENT_ACLMQ_HPP_ */
