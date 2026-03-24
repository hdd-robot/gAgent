/*
 * test_concurrent_send.cpp — Thread-safety de PushCache
 *
 * Scénario : un agent "sender" a N behaviours qui envoient tous vers
 * le même agent "collector" en même temps (threads concurrents).
 * Vérifie que tous les messages sont reçus sans corruption.
 *
 * Assertions :
 *   - collector a reçu exactement N*MSG_PER_THREAD messages
 *   - aucun message corrompu (parse OK)
 *   - aucun timeout côté collector
 */

#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
#include <gagent/messaging/AclMQ.hpp>
#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/messaging/AgentIdentifier.hpp>

#include <iostream>
#include <string>
#include <atomic>
#include <sys/mman.h>
#include <cstring>

using namespace gagent;
using namespace gagent::messaging;

static constexpr int N_THREADS    = 4;   // behaviours concurrents dans sender
static constexpr int MSG_PER_BEH  = 5;   // messages par behaviour
static constexpr int TOTAL        = N_THREADS * MSG_PER_BEH;

// ── Données partagées ─────────────────────────────────────────────────────────

struct Shared {
    int received;    // messages bien reçus par collector
    int parse_ok;    // messages dont le parse a réussi
    int timeouts;    // timeouts côté collector
};

static Shared* g_shared = nullptr;

static void init_shared() {
    g_shared = static_cast<Shared*>(
        mmap(nullptr, sizeof(Shared),
             PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    memset(g_shared, 0, sizeof(Shared));
}

// ── Behaviour expéditeur ──────────────────────────────────────────────────────
// Chaque instance tourne dans son propre thread (Agent::exthread).
// Tous envoient vers "collector" en même temps.

class SenderBehaviour : public Behaviour {
    int id_;
    int count_ = 0;
public:
    SenderBehaviour(Agent* ag, int id) : Behaviour(ag), id_(id) {}

    void action() override {
        ACLMessage msg(ACLMessage::Performative::INFORM);
        msg.setSender(AgentIdentifier{"sender"});
        msg.setContent("beh=" + std::to_string(id_) +
                       ";seq=" + std::to_string(count_));
        msg.setConversationId("concurrent-test");
        acl_send("collector", msg);
        count_++;
    }

    bool done() override { return count_ >= MSG_PER_BEH; }
};

// ── Behaviour collecteur ──────────────────────────────────────────────────────

class CollectorBehaviour : public Behaviour {
    int count_ = 0;
public:
    explicit CollectorBehaviour(Agent* ag) : Behaviour(ag) {}

    void onStart() override { acl_bind("collector"); }

    void action() override {
        auto opt = acl_receive("collector", 2000);
        if (!opt) {
            g_shared->timeouts++;
            // Si on a déjà reçu tous les messages attendus, on s'arrête
            if (g_shared->received >= TOTAL) { count_ = TOTAL; }
            return;
        }
        g_shared->received++;
        if (opt->getPerformative() == ACLMessage::Performative::INFORM)
            g_shared->parse_ok++;
        count_++;
    }

    bool done() override { return count_ >= TOTAL; }
};

// ── Agents ────────────────────────────────────────────────────────────────────

class SenderAgent : public Agent {
public:
    void setup() override {
        for (int i = 0; i < N_THREADS; i++)
            addBehaviour(new SenderBehaviour(this, i));
    }
    void takeDown() override { acl_unlink("sender"); }
};

class CollectorAgent : public Agent {
public:
    void setup()    override { addBehaviour(new CollectorBehaviour(this)); }
    void takeDown() override { acl_unlink("collector"); }
};

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "======================================================\n"
              << " Thread-safety PushCache — " << N_THREADS
              << " threads × " << MSG_PER_BEH << " msgs = " << TOTAL << " total\n"
              << "======================================================\n\n";

    init_shared();
    acl_unlink("sender");
    acl_unlink("collector");

    AgentCore::initAgentSystem();

    CollectorAgent collector;
    SenderAgent    sender;

    collector.init();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    sender.init();

    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();

    // ── Vérifications ─────────────────────────────────────────────────────────

    int ok = 0, fail = 0;
    auto check = [&](bool cond, const std::string& label) {
        std::cout << "  [" << (cond ? "OK  " : "FAIL") << "] " << label << "\n";
        cond ? ok++ : fail++;
    };

    std::cout << "=== Résultat ===\n";
    std::cout << "  messages reçus     : " << g_shared->received  << " / " << TOTAL << "\n";
    std::cout << "  messages parse OK  : " << g_shared->parse_ok  << "\n";
    std::cout << "  timeouts collector : " << g_shared->timeouts   << "\n\n";

    check(g_shared->received == TOTAL,
          "tous les messages reçus (" + std::to_string(TOTAL) + ")");
    check(g_shared->parse_ok == TOTAL,
          "aucun message corrompu");
    check(g_shared->timeouts == 0,
          "aucun timeout");

    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << ok << " OK, " << fail << " FAIL\n\n";

    if (fail == 0) { std::cout << "[OK] PushCache thread-safe\n"; return 0; }
    std::cout << "[FAIL] Erreurs détectées\n";
    return 1;
}
