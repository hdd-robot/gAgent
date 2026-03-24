/*
 * two_agents_acl.cpp — Test du transport ZeroMQ (AclMQ)
 *
 * Scénario : 3 échanges REQUEST/INFORM consécutifs entre Alice et Bob.
 * But principal : valider que le PushCache réutilise les connexions
 * (pas de slow-joiner sur les messages 2 et 3).
 *
 *   Alice → REQUEST "compute:1" → Bob → INFORM "1" → Alice
 *   Alice → REQUEST "compute:2" → Bob → INFORM "2" → Alice
 *   Alice → REQUEST "compute:3" → Bob → INFORM "3" → Alice
 *
 * Assertions vérifiées :
 *   - Bob a traité 3 requêtes
 *   - Alice a reçu 3 INFORM
 *   - Contenu du 3e INFORM = "3"
 *   - Aucun timeout
 */

#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
#include <gagent/messaging/AclMQ.hpp>
#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/messaging/AgentIdentifier.hpp>

#include <iostream>
#include <string>
#include <sys/mman.h>
#include <cstring>

using namespace gagent;
using namespace gagent::messaging;

// ── Données partagées ─────────────────────────────────────────────────────────

struct Shared {
    int  bob_handled;      // nombre de requêtes traitées par Bob
    int  alice_received;   // nombre d'INFORM reçus par Alice
    int  timeouts;         // nombre de timeouts côté Alice
    char last_content[32]; // contenu du dernier INFORM reçu
};

static Shared* g_shared = nullptr;

static void init_shared() {
    g_shared = static_cast<Shared*>(
        mmap(nullptr, sizeof(Shared),
             PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    memset(g_shared, 0, sizeof(Shared));
}

// ── Comportement d'Alice : N cycles REQUEST→INFORM séquentiels ───────────────

class AliceBehaviour : public Behaviour {
    static constexpr int N = 3;
    int  sent_     = 0;
    bool waiting_  = false;
public:
    explicit AliceBehaviour(Agent* ag) : Behaviour(ag) {}

    void onStart() override { acl_bind("alice"); }

    void action() override {
        if (!waiting_) {
            // Envoyer la prochaine requête
            sent_++;
            ACLMessage req(ACLMessage::Performative::REQUEST);
            req.setSender(AgentIdentifier{"alice"});
            req.setContent("compute:" + std::to_string(sent_));
            req.setConversationId("multi-" + std::to_string(sent_));
            acl_send("bob", req);
            waiting_ = true;
        } else {
            // Attendre la réponse
            auto opt = acl_receive("alice", 3000);
            if (!opt) {
                g_shared->timeouts++;
                waiting_ = false;
                return;
            }
            if (opt->getPerformative() == ACLMessage::Performative::INFORM) {
                g_shared->alice_received++;
                strncpy(g_shared->last_content, opt->getContent().c_str(),
                        sizeof(g_shared->last_content) - 1);
            }
            waiting_ = false;
        }
    }

    bool done() override {
        return sent_ >= N && !waiting_;
    }
};

// ── Comportement de Bob : répond à N requêtes puis s'arrête ──────────────────

class BobBehaviour : public Behaviour {
    static constexpr int N = 3;
    int count_ = 0;
public:
    explicit BobBehaviour(Agent* ag) : Behaviour(ag) {}

    void onStart() override { acl_bind("bob"); }

    void action() override {
        auto opt = acl_receive("bob", 3000);
        if (!opt) return;

        const ACLMessage& msg = *opt;
        if (msg.getPerformative() != ACLMessage::Performative::REQUEST) return;

        // Extraire le numéro et répondre avec ce numéro
        std::string content = msg.getContent(); // "compute:N"
        std::string val = content.substr(content.find(':') + 1);

        ACLMessage reply = msg.createReply(ACLMessage::Performative::INFORM);
        reply.setSender(AgentIdentifier{"bob"});
        reply.setContent(val);
        acl_send(msg.getSender().name, reply);

        g_shared->bob_handled++;
        count_++;
    }

    bool done() override { return count_ >= N; }
};

// ── Agents ────────────────────────────────────────────────────────────────────

class AliceAgent : public Agent {
public:
    void setup()    override { addBehaviour(new AliceBehaviour(this)); }
    void takeDown() override { acl_unlink("alice"); }
};

class BobAgent : public Agent {
public:
    void setup()    override { addBehaviour(new BobBehaviour(this)); }
    void takeDown() override { acl_unlink("bob"); }
};

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "======================================================\n"
              << " Transport ZeroMQ — Alice ↔ Bob (3 cycles REQUEST/INFORM)\n"
              << "======================================================\n\n";

    init_shared();

    acl_unlink("alice");
    acl_unlink("bob");

    AgentCore::initAgentSystem();

    BobAgent   bob;
    AliceAgent alice;

    bob.init();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    alice.init();

    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();

    // ── Vérifications ─────────────────────────────────────────────────────────

    int ok = 0, fail = 0;
    auto check = [&](bool cond, const std::string& label) {
        std::cout << "  [" << (cond ? "OK  " : "FAIL") << "] " << label << "\n";
        cond ? ok++ : fail++;
    };

    std::cout << "=== Résultat ===\n";
    std::cout << "  requêtes traitées (Bob)  : " << g_shared->bob_handled    << "\n";
    std::cout << "  INFORM reçus (Alice)     : " << g_shared->alice_received  << "\n";
    std::cout << "  timeouts                 : " << g_shared->timeouts        << "\n";
    std::cout << "  dernier contenu INFORM   : " << g_shared->last_content    << "\n\n";

    check(g_shared->bob_handled   == 3, "Bob a traité 3 requêtes");
    check(g_shared->alice_received == 3, "Alice a reçu 3 INFORM");
    check(std::string(g_shared->last_content) == "3", "3e INFORM contient \"3\"");
    check(g_shared->timeouts      == 0, "aucun timeout");

    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << ok << " OK, " << fail << " FAIL\n\n";

    if (fail == 0) {
        std::cout << "[OK] Transport ZeroMQ fonctionne\n";
        return 0;
    }
    std::cout << "[FAIL] Erreurs détectées\n";
    return 1;
}
