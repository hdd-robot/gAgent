/*
 * MockTransport.hpp — Transport en mémoire pour les tests unitaires
 *
 * Permet de tester les protocoles FIPA (Request, ContractNet, Subscribe)
 * sans fork, sans ZeroMQ, sans IPC et sans timeouts réels.
 *
 * Usage :
 *
 *   auto bus = std::make_shared<MockBus>();
 *
 *   StubAgent alice; alice.setTransport(std::make_shared<MockTransport>(bus));
 *   StubAgent bob;   bob.setTransport(std::make_shared<MockTransport>(bus));
 *
 *   // Instancier les behaviours directement, les piloter manuellement :
 *   MyInitiator init(&alice, ...);
 *   MyServer    serv(&bob, ...);
 *   init.onStart();  serv.onStart();
 *   init.action();   // envoie REQUEST dans le bus
 *   serv.action();   // reçoit REQUEST, envoie INFORM
 *   init.action();   // reçoit INFORM → handleInform()
 */

#ifndef GAGENT_MOCKTRANSPORT_HPP_
#define GAGENT_MOCKTRANSPORT_HPP_

#include <gagent/messaging/ITransport.hpp>
#include <map>
#include <queue>
#include <mutex>
#include <memory>

namespace gagent {
namespace messaging {

// ── MockBus ───────────────────────────────────────────────────────────────────
// File d'attente en mémoire partagée entre tous les MockTransport d'un test.

class MockBus {
public:
    void deliver(const std::string& to, const ACLMessage& msg) {
        std::lock_guard<std::mutex> lock(mtx_);
        queues_[to].push(msg);
    }

    std::optional<ACLMessage> poll(const std::string& name) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = queues_.find(name);
        if (it == queues_.end() || it->second.empty()) return std::nullopt;
        ACLMessage msg = it->second.front();
        it->second.pop();
        return msg;
    }

    void clear(const std::string& name) {
        std::lock_guard<std::mutex> lock(mtx_);
        queues_.erase(name);
    }

    /** Nombre de messages en attente pour <name> (utile pour les assertions). */
    int pending(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = queues_.find(name);
        if (it == queues_.end()) return 0;
        return static_cast<int>(it->second.size());
    }

private:
    mutable std::mutex mtx_;
    std::map<std::string, std::queue<ACLMessage>> queues_;
};

// ── MockTransport ─────────────────────────────────────────────────────────────

class MockTransport : public ITransport {
public:
    explicit MockTransport(std::shared_ptr<MockBus> bus)
        : bus_(std::move(bus)) {}

    /** Enregistre la queue (no-op : le bus crée les queues à la demande). */
    std::string bind(const std::string& name) override {
        return "mock://" + name;
    }

    /** Dépose le message dans la queue du destinataire. */
    bool send(const std::string& to, const ACLMessage& msg) override {
        bus_->deliver(to, msg);
        return true;
    }

    /**
     * Dépile un message de la queue <name>.
     * timeout_ms est ignoré : retourne std::nullopt immédiatement si vide.
     * Les tests doivent piloter l'ordre des appels action() pour garantir
     * qu'un message est présent avant de le recevoir.
     */
    std::optional<ACLMessage> receive(const std::string& name,
                                      int /*timeout_ms*/) override {
        return bus_->poll(name);
    }

    void unlink(const std::string& name) override {
        bus_->clear(name);
    }

    void flush() override {}

    /** Accès au bus pour les assertions dans les tests. */
    MockBus& bus() { return *bus_; }

private:
    std::shared_ptr<MockBus> bus_;
};

} // namespace messaging
} // namespace gagent

#endif /* GAGENT_MOCKTRANSPORT_HPP_ */
