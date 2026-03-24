/*
 * test_request.cpp — Test du protocole FIPA Request
 *
 * Scénario : un agent "client" demande à "server" de calculer 3+4.
 *            server répond INFORM avec "7".
 *
 * Flux :
 *   client → REQUEST "add(3,4)" → server
 *   server → INFORM  "7"        → client
 */

#include <gagent/core/AgentCore.hpp>
#include <gagent/env/Environnement.hpp>
#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/protocols/Request.hpp>
#include <gagent/messaging/AclMQ.hpp>

#include <iostream>
#include <thread>
#include <chrono>
#include <sys/mman.h>
#include <cstring>

using namespace gagent;
using namespace gagent::protocols;
using namespace gagent::messaging;

// ── Compteurs partagés entre processus (mmap MAP_SHARED) ─────────────────────

struct Shared {
    int  inform_received;
    int  request_handled;
    char result[64];
};

static Shared* g_shared = nullptr;

static void init_shared() {
    g_shared = static_cast<Shared*>(mmap(nullptr, sizeof(Shared),
                                         PROT_READ|PROT_WRITE,
                                         MAP_SHARED|MAP_ANONYMOUS, -1, 0));
    g_shared->inform_received = 0;
    g_shared->request_handled = 0;
    memset(g_shared->result, 0, sizeof(g_shared->result));
}

// ── Comportements ─────────────────────────────────────────────────────────────

class CalcRequester : public RequestInitiator {
public:
    CalcRequester(Agent* ag)
        : RequestInitiator(ag, "client", "server", "add(3,4)", "math") {}

    void handleInform(const ACLMessage& msg) override {
        strncpy(g_shared->result, msg.getContent().c_str(),
                sizeof(g_shared->result) - 1);
        g_shared->inform_received++;
    }
    void handleRefuse(const ACLMessage&) override {
        g_shared->inform_received++;
    }
    void handleTimeout() override {
        std::cerr << "[client] timeout !\n";
        g_shared->inform_received++;
    }
};

// Serveur : répond à une REQUEST, puis s'arrête après N requêtes traitées
class CalcServer : public RequestParticipant {
    int max_;
    int count_ = 0;
public:
    CalcServer(Agent* ag, int max_requests = 1)
        : RequestParticipant(ag, "server", 300)
        , max_(max_requests) {}

    ACLMessage handleRequest(const ACLMessage& req) override {
        g_shared->request_handled++;
        count_++;
        ACLMessage reply = req.createReply(ACLMessage::Performative::INFORM);
        reply.setSender(AgentIdentifier{"server"});
        reply.setContent("7");   // 3 + 4
        return reply;
    }

    bool done() override { return count_ >= max_; }
};

// ── Agents ────────────────────────────────────────────────────────────────────

class ClientAgent : public Agent {
public:
    void setup() override {
        addBehaviour(new CalcRequester(this));
    }
    void takeDown() override { acl_unlink("client"); }
};

class ServerAgent : public Agent {
public:
    void setup() override {
        addBehaviour(new CalcServer(this, 1));
    }
    void takeDown() override { acl_unlink("server"); }
};

// ── Environnement factice ─────────────────────────────────────────────────────

class DummyEnv : public Environnement {
public:
    void init_env()      override {}
    void link_attribut() override {}
    void event_loop()    override {}
};

// ── main ──────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "======================================================\n"
              << " FIPA Request — calcul distribué\n"
              << "======================================================\n\n";

    init_shared();

    acl_unlink("client");
    acl_unlink("server");

    AgentCore::initAgentSystem();

    DummyEnv env;
    AgentCore::initEnvironnementSystem(env);

    ServerAgent server;
    ClientAgent client;

    server.init();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client.init();

    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();

    std::cout << "\n=== Résultat ===\n";
    std::cout << "  requêtes traitées : " << g_shared->request_handled << "\n";
    std::cout << "  réponses reçues   : " << g_shared->inform_received  << "\n";
    std::cout << "  contenu INFORM    : " << g_shared->result           << "\n\n";

    int ok = 0, fail = 0;
    auto check = [&](bool cond, const std::string& label) {
        std::cout << "  [" << (cond ? "OK  " : "FAIL") << "] " << label << "\n";
        cond ? ok++ : fail++;
    };

    check(g_shared->request_handled >= 1, "server a traité la requête");
    check(g_shared->inform_received == 1,  "client a reçu INFORM");
    check(std::string(g_shared->result) == "7", "résultat correct (7)");

    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << ok << " OK, " << fail << " FAIL\n\n";

    if (fail == 0) {
        std::cout << "[OK] Request fonctionne\n";
        return 0;
    }
    std::cout << "[FAIL] Erreurs détectées\n";
    return 1;
}
