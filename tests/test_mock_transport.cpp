/*
 * test_mock_transport.cpp — Tests unitaires des protocoles FIPA via MockTransport
 *
 * Aucun fork, aucun ZeroMQ, aucun IPC, aucun timeout réel.
 * Les state machines sont pilotées manuellement (onStart + action).
 *
 * Scénarios :
 *   1. Request sync       : REQUEST → INFORM
 *   2. Request AGREE      : REQUEST → AGREE → INFORM  (FIPA SC00026H §3.4)
 *   3. Request refuse     : REQUEST → REFUSE
 *   4. ContractNet        : CFP → PROPOSE x2 → ACCEPT/REJECT → INFORM
 */

#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/messaging/MockTransport.hpp>
#include <gagent/protocols/Request.hpp>
#include <gagent/protocols/ContractNet.hpp>

#include <iostream>
#include <string>
#include <memory>

using namespace gagent;
using namespace gagent::messaging;
using namespace gagent::protocols;

// ── StubAgent ─────────────────────────────────────────────────────────────────
// Agent minimal pour les tests : pas d'init(), pas de fork, juste un transport.

class StubAgent : public Agent {
public:
    explicit StubAgent(std::shared_ptr<ITransport> t) {
        setTransport(std::move(t));
    }
    void setup() override {}
};

// ── Helpers ───────────────────────────────────────────────────────────────────

static int g_ok   = 0;
static int g_fail = 0;

static void check(bool cond, const std::string& label)
{
    std::cout << "  [" << (cond ? "OK  " : "FAIL") << "] " << label << "\n";
    cond ? g_ok++ : g_fail++;
}

// ── Scénario 1 : Request sync (REQUEST → INFORM) ──────────────────────────────

static void test_request_sync()
{
    std::cout << "\n--- Scénario 1 : Request sync (REQUEST → INFORM) ---\n";

    auto bus = std::make_shared<MockBus>();
    StubAgent alice(std::make_shared<MockTransport>(bus));
    StubAgent bob  (std::make_shared<MockTransport>(bus));

    bool inform_received = false;
    bool request_handled = false;

    struct Requester : RequestInitiator {
        bool& flag_;
        Requester(Agent* ag, bool& f)
            : RequestInitiator(ag, "alice", "bob", "add(3,4)", "math", 500)
            , flag_(f) {}
        void handleInform(const ACLMessage&) override { flag_ = true; }
    };

    struct Server : RequestParticipant {
        bool& flag_;
        int   count_ = 0;
        Server(Agent* ag, bool& f) : RequestParticipant(ag, "bob", 100), flag_(f) {}
        ACLMessage handleRequest(const ACLMessage& req) override {
            flag_ = true;
            count_++;
            auto r = req.createReply(ACLMessage::Performative::INFORM);
            r.setSender(AgentIdentifier{"bob"});
            r.setContent("7");
            return r;
        }
        bool done() override { return count_ >= 1; }
    };

    Requester req(&alice, inform_received);
    Server    srv(&bob,   request_handled);

    req.onStart();  // bind("alice")
    srv.onStart();  // bind("bob")

    req.action();   // SEND_REQUEST → envoie REQUEST à "bob"
    srv.action();   // reçoit REQUEST → envoie INFORM à "alice"
    req.action();   // WAIT_RESPONSE → reçoit INFORM → handleInform()

    check(request_handled,  "bob a traité la requête");
    check(inform_received,  "alice a reçu INFORM");
    check(req.done(),       "requester terminé");
    check(srv.done(),       "server terminé");
}

// ── Scénario 2 : Request AGREE + INFORM ───────────────────────────────────────

static void test_request_agree()
{
    std::cout << "\n--- Scénario 2 : AGREE + INFORM (FIPA SC00026H §3.4) ---\n";

    auto bus = std::make_shared<MockBus>();
    StubAgent alice(std::make_shared<MockTransport>(bus));
    StubAgent bob  (std::make_shared<MockTransport>(bus));

    bool agree_received  = false;
    bool inform_received = false;

    struct Requester : RequestInitiator {
        bool& agree_;
        bool& inform_;
        Requester(Agent* ag, bool& a, bool& i)
            : RequestInitiator(ag, "alice2", "bob2", "long-op", "", 500)
            , agree_(a), inform_(i) {}
        void handleAgree (const ACLMessage&) override { agree_  = true; }
        void handleInform(const ACLMessage&) override { inform_ = true; }
    };

    struct SlowServer : RequestParticipant {
        int count_ = 0;
        SlowServer(Agent* ag) : RequestParticipant(ag, "bob2", 100) {}
        bool prepareAgree(const ACLMessage&) override { return true; }  // AGREE d'abord
        ACLMessage handleRequest(const ACLMessage& req) override {
            count_++;
            auto r = req.createReply(ACLMessage::Performative::INFORM);
            r.setSender(AgentIdentifier{"bob2"});
            r.setContent("long-op done");
            return r;
        }
        bool done() override { return count_ >= 1; }
    };

    Requester  req(&alice, agree_received, inform_received);
    SlowServer srv(&bob);

    req.onStart();
    srv.onStart();

    req.action();   // envoie REQUEST
    srv.action();   // reçoit REQUEST → envoie AGREE puis INFORM
    req.action();   // reçoit AGREE → handleAgree(), reste en WAIT_RESPONSE
    req.action();   // reçoit INFORM → handleInform() → done

    check(agree_received,  "alice a reçu AGREE");
    check(inform_received, "alice a reçu INFORM après AGREE");
    check(req.done(),      "requester terminé");
    check(srv.done(),      "server terminé");
}

// ── Scénario 3 : Request refuse ───────────────────────────────────────────────

static void test_request_refuse()
{
    std::cout << "\n--- Scénario 3 : Request refuse (REQUEST → REFUSE) ---\n";

    auto bus = std::make_shared<MockBus>();
    StubAgent alice(std::make_shared<MockTransport>(bus));
    StubAgent bob  (std::make_shared<MockTransport>(bus));

    bool refuse_received = false;
    bool inform_received = false;

    struct Requester : RequestInitiator {
        bool& refuse_;
        bool& inform_;
        Requester(Agent* ag, bool& r, bool& i)
            : RequestInitiator(ag, "alice3", "bob3", "forbidden", "", 500)
            , refuse_(r), inform_(i) {}
        void handleRefuse(const ACLMessage&) override { refuse_ = true; }
        void handleInform(const ACLMessage&) override { inform_ = true; }
    };

    struct RefuseServer : RequestParticipant {
        int count_ = 0;
        RefuseServer(Agent* ag) : RequestParticipant(ag, "bob3", 100) {}
        ACLMessage handleRequest(const ACLMessage& req) override {
            count_++;
            auto r = req.createReply(ACLMessage::Performative::REFUSE);
            r.setSender(AgentIdentifier{"bob3"});
            r.setContent("non autorisé");
            return r;
        }
        bool done() override { return count_ >= 1; }
    };

    Requester   req(&alice, refuse_received, inform_received);
    RefuseServer srv(&bob);

    req.onStart();
    srv.onStart();

    req.action();   // envoie REQUEST
    srv.action();   // reçoit REQUEST → envoie REFUSE
    req.action();   // reçoit REFUSE → handleRefuse() → done

    check(refuse_received,  "alice a reçu REFUSE");
    check(!inform_received, "alice n'a PAS reçu INFORM");
    check(req.done(),       "requester terminé");
    check(srv.done(),       "server terminé");
}

// ── Scénario 4 : ContractNet complet ─────────────────────────────────────────

static void test_contract_net()
{
    std::cout << "\n--- Scénario 4 : ContractNet (CFP → PROPOSE x2 → ACCEPT/REJECT → INFORM) ---\n";

    auto bus = std::make_shared<MockBus>();
    StubAgent manager_ag(std::make_shared<MockTransport>(bus));
    StubAgent bob_ag    (std::make_shared<MockTransport>(bus));
    StubAgent carol_ag  (std::make_shared<MockTransport>(bus));

    std::string accepted_name;
    bool inform_received = false;

    // Initiateur : accepte l'offre la moins chère
    struct Manager : ContractNetInitiator {
        std::string& accepted_;
        bool&        inform_;

        Manager(Agent* ag, ACLMessage cfp,
                std::vector<AgentIdentifier> parts,
                std::string& acc, bool& inf)
            : ContractNetInitiator(ag, "manager", cfp, parts, 500, 500)
            , accepted_(acc), inform_(inf) {}

        std::vector<std::string> selectProposals(
                const std::vector<ACLMessage>& proposals) override
        {
            // Choisir l'offre avec le coût le plus bas
            const ACLMessage* best = nullptr;
            int best_cost = INT_MAX;
            for (auto& p : proposals) {
                int cost = std::stoi(p.getContent());
                if (cost < best_cost) { best_cost = cost; best = &p; }
            }
            if (!best) return {};
            accepted_ = best->getSender().name;
            return { accepted_ };
        }

        void handleInform(const ACLMessage&) override { inform_ = true; }
    };

    // Participant : propose un coût fixe
    struct Worker : ContractNetParticipant {
        int cost_;
        Worker(Agent* ag, const std::string& name, int cost)
            : ContractNetParticipant(ag, name, 500), cost_(cost) {}

        ACLMessage prepareProposal(const ACLMessage& cfp) override {
            ACLMessage prop(ACLMessage::Performative::PROPOSE);
            prop.setContent(std::to_string(cost_));
            return prop;
        }
        ACLMessage executeTask(const ACLMessage&) override {
            ACLMessage r(ACLMessage::Performative::INFORM);
            r.setContent("done");
            return r;
        }
    };

    ACLMessage cfp(ACLMessage::Performative::CFP);
    cfp.setContent("do-task");
    std::vector<AgentIdentifier> parts = { AgentIdentifier{"bob"}, AgentIdentifier{"carol"} };

    Manager manager(&manager_ag, cfp, parts, accepted_name, inform_received);
    Worker  bob    (&bob_ag,   "bob",   10);
    Worker  carol  (&carol_ag, "carol",  5);  // carol moins chère

    manager.onStart();
    bob.onStart();
    carol.onStart();

    // Étape 1 : manager envoie CFP à bob et carol
    manager.action();   // SEND_CFP → envoie CFP × 2

    // Étape 2 : participants reçoivent le CFP et proposent
    bob.action();       // WAIT_CFP → reçoit CFP → envoie PROPOSE(10)
    carol.action();     // WAIT_CFP → reçoit CFP → envoie PROPOSE(5)

    // Étape 3 : manager collecte les offres
    manager.action();   // WAIT_PROPOSALS → reçoit PROPOSE de bob (responded=1)
    manager.action();   // WAIT_PROPOSALS → reçoit PROPOSE de carol (responded=2)
                        // → HANDLE_PROPOSALS automatique
    manager.action();   // HANDLE_PROPOSALS → envoie ACCEPT à carol, REJECT à bob
                        // → WAIT_RESULTS

    // Étape 4 : participants traitent ACCEPT/REJECT
    bob.action();       // WAIT_DECISION → reçoit REJECT → done
    carol.action();     // WAIT_DECISION → reçoit ACCEPT → exécute → envoie INFORM

    // Étape 5 : manager reçoit INFORM
    manager.action();   // WAIT_RESULTS → reçoit INFORM → handleInform() → state=DONE
    manager.action();   // DONE → done_ = true

    check(accepted_name == "carol",  "carol sélectionnée (offre la moins chère : 5)");
    check(inform_received,           "manager a reçu INFORM de carol");
    check(manager.done(),            "manager terminé");
    check(bob.done(),                "bob terminé (REJECT)");
    check(carol.done(),              "carol terminée (ACCEPT + INFORM envoyé)");
}

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "======================================================\n"
              << " Tests unitaires MockTransport (sans fork, sans ZMQ)\n"
              << "======================================================\n";

    test_request_sync();
    test_request_agree();
    test_request_refuse();
    test_contract_net();

    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << g_ok << " OK, " << g_fail << " FAIL\n\n";

    if (g_fail == 0) { std::cout << "[OK] MockTransport fonctionne\n"; return 0; }
    std::cout << "[FAIL] Erreurs détectées\n";
    return 1;
}
