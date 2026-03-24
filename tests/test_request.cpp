/*
 * test_request.cpp — Test du protocole FIPA Request
 *
 * Cinq scénarios :
 *
 *   1. Succès       : client → REQUEST → server  → INFORM "7"   → client
 *   2. Refus        : client → REQUEST → server2 → REFUSE        → client
 *   3. Timeout      : client → REQUEST → (pas de serveur)        → timeout
 *   4. Failure      : client → REQUEST → server4 → FAILURE       → client
 *   5. AGREE+INFORM : client → REQUEST → server5 → AGREE → INFORM → client
 */

#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
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

// ── Données partagées ─────────────────────────────────────────────────────────

struct Shared {
    int  request_handled;
    int  inform_received;
    int  refuse_received;
    int  timeout_received;
    int  failure_received;
    int  agree_received;
    char result[64];
};

static Shared* g_shared = nullptr;

static void init_shared() {
    g_shared = static_cast<Shared*>(
        mmap(nullptr, sizeof(Shared),
             PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    memset(g_shared, 0, sizeof(Shared));
}

// ── Scénario 1 : succès ───────────────────────────────────────────────────────

class CalcRequester : public RequestInitiator {
public:
    CalcRequester(Agent* ag)
        : RequestInitiator(ag, "client", "server", "add(3,4)", "math") {}

    void handleInform(const ACLMessage& msg) override {
        strncpy(g_shared->result, msg.getContent().c_str(),
                sizeof(g_shared->result) - 1);
        g_shared->inform_received++;
    }
    void handleTimeout() override { g_shared->timeout_received++; }
};

class CalcServer : public RequestParticipant {
    int count_ = 0;
public:
    CalcServer(Agent* ag) : RequestParticipant(ag, "server", 300) {}

    ACLMessage handleRequest(const ACLMessage& req) override {
        g_shared->request_handled++;
        count_++;
        auto reply = req.createReply(ACLMessage::Performative::INFORM);
        reply.setSender(AgentIdentifier{"server"});
        reply.setContent("7");
        return reply;
    }
    bool done() override { return count_ >= 1; }
};

class ClientAgent : public Agent {
public:
    void setup()    override { addBehaviour(new CalcRequester(this)); }
    void takeDown() override { acl_unlink("client"); }
};

class ServerAgent : public Agent {
public:
    void setup()    override { addBehaviour(new CalcServer(this)); }
    void takeDown() override { acl_unlink("server"); }
};

// ── Scénario 2 : refus ────────────────────────────────────────────────────────

class RefuseRequester : public RequestInitiator {
public:
    RefuseRequester(Agent* ag)
        : RequestInitiator(ag, "client2", "server2", "forbidden", "", 5000) {}

    void handleRefuse(const ACLMessage& /*msg*/) override { g_shared->refuse_received++; }
    void handleTimeout() override { g_shared->timeout_received++; }
};

class RefuseServer : public RequestParticipant {
    int count_ = 0;
public:
    RefuseServer(Agent* ag) : RequestParticipant(ag, "server2", 300) {}

    ACLMessage handleRequest(const ACLMessage& req) override {
        g_shared->request_handled++;
        count_++;
        auto reply = req.createReply(ACLMessage::Performative::REFUSE);
        reply.setSender(AgentIdentifier{"server2"});
        reply.setContent("opération non autorisée");
        return reply;
    }
    bool done() override { return count_ >= 1; }
};

class Client2Agent : public Agent {
public:
    void setup()    override { addBehaviour(new RefuseRequester(this)); }
    void takeDown() override { acl_unlink("client2"); }
};

class Server2Agent : public Agent {
public:
    void setup()    override { addBehaviour(new RefuseServer(this)); }
    void takeDown() override { acl_unlink("server2"); }
};

// ── Scénario 3 : timeout ──────────────────────────────────────────────────────

class TimeoutRequester : public RequestInitiator {
public:
    TimeoutRequester(Agent* ag)
        : RequestInitiator(ag, "client3", "ghost", "ping", "", 600) {}

    void handleTimeout() override { g_shared->timeout_received++; }
};

class Client3Agent : public Agent {
public:
    void setup()    override { addBehaviour(new TimeoutRequester(this)); }
    void takeDown() override { acl_unlink("client3"); }
};

// ── Scénario 4 : failure ──────────────────────────────────────────────────────

class FailureRequester : public RequestInitiator {
public:
    FailureRequester(Agent* ag)
        : RequestInitiator(ag, "client4", "server4", "risky-op", "", 5000) {}

    void handleFailure(const ACLMessage& /*msg*/) override { g_shared->failure_received++; }
    void handleTimeout() override { g_shared->timeout_received++; }
};

class FailureServer : public RequestParticipant {
    int count_ = 0;
public:
    FailureServer(Agent* ag) : RequestParticipant(ag, "server4", 300) {}

    ACLMessage handleRequest(const ACLMessage& req) override {
        g_shared->request_handled++;
        count_++;
        auto reply = req.createReply(ACLMessage::Performative::FAILURE);
        reply.setSender(AgentIdentifier{"server4"});
        reply.setContent("exception: opération échouée");
        return reply;
    }
    bool done() override { return count_ >= 1; }
};

class Client4Agent : public Agent {
public:
    void setup()    override { addBehaviour(new FailureRequester(this)); }
    void takeDown() override { acl_unlink("client4"); }
};

class Server4Agent : public Agent {
public:
    void setup()    override { addBehaviour(new FailureServer(this)); }
    void takeDown() override { acl_unlink("server4"); }
};

// ── Scénario 5 : AGREE + INFORM ──────────────────────────────────────────────

class AgreeRequester : public RequestInitiator {
public:
    AgreeRequester(Agent* ag)
        : RequestInitiator(ag, "client5", "server5", "long-op", "", 5000) {}

    void handleAgree  (const ACLMessage& /*msg*/) override { g_shared->agree_received++;   }
    void handleInform (const ACLMessage& /*msg*/) override { g_shared->inform_received++;  }
    void handleTimeout()                          override { g_shared->timeout_received++; }
};

// Serveur qui envoie AGREE d'abord, puis INFORM après un délai
class AgreeServer : public Behaviour {
    enum State { WAIT_REQ, SEND_INFORM } state_ = WAIT_REQ;
    ACLMessage pending_;
    int count_ = 0;
public:
    AgreeServer(Agent* ag) : Behaviour(ag) {}

    void onStart() override { acl_bind("server5"); }

    void action() override {
        if (state_ == WAIT_REQ) {
            auto opt = acl_receive("server5", 3000);
            if (!opt) return;
            if (opt->getPerformative() != ACLMessage::Performative::REQUEST) return;
            pending_ = *opt;

            // Envoyer AGREE immédiatement
            ACLMessage agree = pending_.createReply(ACLMessage::Performative::AGREE);
            agree.setSender(AgentIdentifier{"server5"});
            acl_send(pending_.getSender().name, agree);

            state_ = SEND_INFORM;

        } else {
            // Simuler un traitement long
            std::this_thread::sleep_for(std::chrono::milliseconds(150));

            ACLMessage inform = pending_.createReply(ACLMessage::Performative::INFORM);
            inform.setSender(AgentIdentifier{"server5"});
            inform.setContent("long-op done");
            acl_send(pending_.getSender().name, inform);

            count_++;
            state_ = WAIT_REQ;
        }
    }

    bool done() override { return count_ >= 1; }
};

class Client5Agent : public Agent {
public:
    void setup()    override { addBehaviour(new AgreeRequester(this)); }
    void takeDown() override { acl_unlink("client5"); }
};

class Server5Agent : public Agent {
public:
    void setup()    override { addBehaviour(new AgreeServer(this)); }
    void takeDown() override { acl_unlink("server5"); }
};

// ── Helpers ───────────────────────────────────────────────────────────────────

static void run_check(bool cond, const std::string& label, int& ok, int& fail)
{
    std::cout << "  [" << (cond ? "OK  " : "FAIL") << "] " << label << "\n";
    cond ? ok++ : fail++;
}

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "======================================================\n"
              << " FIPA Request — cinq scénarios\n"
              << "======================================================\n\n";

    init_shared();

    for (auto n : {"client","server","client2","server2","client3","ghost",
                   "client4","server4","client5","server5"})
        acl_unlink(n);

    AgentCore::initAgentSystem();

    int ok = 0, fail = 0;

    // ── Scénario 1 : succès ───────────────────────────────────────────────────
    std::cout << "--- Scénario 1 : succès (REQUEST → INFORM) ---\n";
    {
        ServerAgent server; ClientAgent client;
        server.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        client.init();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
    }
    run_check(g_shared->request_handled >= 1,         "server a traité la requête",         ok, fail);
    run_check(g_shared->inform_received  == 1,         "client a reçu INFORM",               ok, fail);
    run_check(std::string(g_shared->result) == "7",   "résultat correct (7)",               ok, fail);
    run_check(g_shared->timeout_received == 0,         "aucun timeout",                      ok, fail);

    // ── Scénario 2 : refus ────────────────────────────────────────────────────
    std::cout << "\n--- Scénario 2 : refus (REQUEST → REFUSE) ---\n";
    {
        int rh_before = g_shared->request_handled;
        Server2Agent server2; Client2Agent client2;
        server2.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        client2.init();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
        run_check(g_shared->request_handled > rh_before, "server2 a traité la requête",     ok, fail);
    }
    run_check(g_shared->refuse_received  == 1,         "client2 a reçu REFUSE",              ok, fail);
    run_check(g_shared->inform_received  == 1,         "INFORM non reçu (scénario refus)",   ok, fail);

    // ── Scénario 3 : timeout ──────────────────────────────────────────────────
    std::cout << "\n--- Scénario 3 : timeout (pas de serveur) ---\n";
    {
        Client3Agent client3;
        client3.init();
        AgentCore::syncAgentSystem();
    }
    run_check(g_shared->timeout_received == 1,         "client3 a reçu un timeout",          ok, fail);
    run_check(g_shared->inform_received  == 1,         "INFORM non reçu (scénario timeout)", ok, fail);

    // ── Scénario 4 : failure ──────────────────────────────────────────────────
    std::cout << "\n--- Scénario 4 : failure (REQUEST → FAILURE) ---\n";
    {
        int rh_before = g_shared->request_handled;
        Server4Agent server4; Client4Agent client4;
        server4.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        client4.init();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
        run_check(g_shared->request_handled > rh_before, "server4 a traité la requête",     ok, fail);
    }
    run_check(g_shared->failure_received == 1,         "client4 a reçu FAILURE",             ok, fail);
    run_check(g_shared->inform_received  == 1,         "INFORM non reçu (scénario failure)", ok, fail);
    run_check(g_shared->timeout_received == 1,         "aucun timeout supplémentaire",       ok, fail);

    // ── Scénario 5 : AGREE + INFORM ───────────────────────────────────────────
    std::cout << "\n--- Scénario 5 : AGREE + INFORM (traitement long) ---\n";
    {
        Server5Agent server5; Client5Agent client5;
        server5.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        client5.init();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
    }
    run_check(g_shared->agree_received  == 1,          "client5 a reçu AGREE",               ok, fail);
    run_check(g_shared->inform_received == 2,          "client5 a reçu INFORM (après AGREE)",ok, fail);
    run_check(g_shared->timeout_received == 1,         "aucun timeout supplémentaire",       ok, fail);

    // ── Bilan ─────────────────────────────────────────────────────────────────
    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << ok << " OK, " << fail << " FAIL\n\n";

    if (fail == 0) { std::cout << "[OK] Request fonctionne\n"; return 0; }
    std::cout << "[FAIL] Erreurs détectées\n";
    return 1;
}
