/*
 * test_contract_net.cpp — Test d'intégration du protocole FIPA Contract Net
 *
 * Deux scénarios :
 *
 *   1. Succès  : donneur CFP → {transp-a 70€, transp-b 85€, transp-c refuse}
 *                donneur accepte transp-a (moins cher)
 *                transp-a exécute et retourne INFORM
 *
 *   2. Tous refusent : donneur CFP → {transp-d refuse, transp-e refuse}
 *                      selectProposals([]) → {} → DONE sans ACCEPT
 *
 * Assertions vérifiées :
 *   Scénario 1 : gagnant=transp-a, 2 PROPOSE, 1 REFUSE, 1 INFORM, contenu OK
 *   Scénario 2 : aucun gagnant, 0 PROPOSE, 2 REFUSE, 0 INFORM
 */

#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
#include <gagent/protocols/ContractNet.hpp>
#include <gagent/messaging/AclMQ.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <cstring>
#include <climits>

using namespace gagent;
using namespace gagent::protocols;
using namespace gagent::messaging;

// ── Données partagées ─────────────────────────────────────────────────────────

struct Shared {
    // Scénario 1
    char winner[32];
    int  proposal_count;
    int  refuse_count;
    int  inform_received;
    char inform_content[128];
    // Scénario 2
    char winner2[32];
    int  proposal_count2;
    int  refuse_count2;
    int  inform_received2;
};

static Shared* g_shared = nullptr;

static void init_shared() {
    g_shared = static_cast<Shared*>(
        mmap(nullptr, sizeof(Shared),
             PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    memset(g_shared, 0, sizeof(Shared));
}

// ── Initiateur générique ──────────────────────────────────────────────────────

class LivraisonInitiator : public ContractNetInitiator {
    char*  winner_buf_;
    int*   proposal_count_;
    int*   refuse_count_;
    int*   inform_received_;
    char*  inform_content_;
public:
    LivraisonInitiator(Agent* ag,
                       const std::string& my_name,
                       const std::vector<AgentIdentifier>& participants,
                       char* winner_buf, int* proposal_count,
                       int* refuse_count, int* inform_received,
                       char* inform_content)
        : ContractNetInitiator(
            ag, my_name,
            []() {
                ACLMessage cfp(ACLMessage::Performative::CFP);
                cfp.setContent("livraison:Paris;poids:5kg;budget:100");
                cfp.setOntology("logistics");
                return cfp;
            }(),
            participants, 4000, 6000)
        , winner_buf_(winner_buf), proposal_count_(proposal_count)
        , refuse_count_(refuse_count), inform_received_(inform_received)
        , inform_content_(inform_content)
    {}

    std::vector<std::string> selectProposals(
            const std::vector<ACLMessage>& proposals) override
    {
        *proposal_count_ = (int)proposals.size();
        std::string best_name;
        int best_price = INT_MAX;

        for (auto& p : proposals) {
            int price = 999;
            auto pos = p.getContent().find("tarif:");
            if (pos != std::string::npos)
                price = std::stoi(p.getContent().substr(pos + 6));
            if (price < best_price) { best_price = price; best_name = p.getSender().name; }
        }

        strncpy(winner_buf_, best_name.c_str(), 31);
        return best_name.empty() ? std::vector<std::string>{} :
                                   std::vector<std::string>{best_name};
    }

    void handleRefuse (const ACLMessage& /*m*/) override { (*refuse_count_)++; }
    void handleInform (const ACLMessage& msg)   override {
        *inform_received_ = 1;
        if (inform_content_)
            strncpy(inform_content_, msg.getContent().c_str(), 127);
    }
    void onEnd() override { this_agent->doDelete(); }
};

// ── Participant générique ─────────────────────────────────────────────────────

class TransporteurParticipant : public ContractNetParticipant {
    std::string name_;
    int  tarif_;
    bool refuse_;
public:
    TransporteurParticipant(Agent* ag, const std::string& name,
                            int tarif, bool refuse = false)
        : ContractNetParticipant(ag, name, 5000)
        , name_(name), tarif_(tarif), refuse_(refuse) {}

    ACLMessage prepareProposal(const ACLMessage& /*cfp*/) override {
        if (refuse_) {
            ACLMessage r(ACLMessage::Performative::REFUSE);
            r.setContent("surchargé");
            return r;
        }
        ACLMessage prop(ACLMessage::Performative::PROPOSE);
        prop.setContent("tarif:" + std::to_string(tarif_));
        return prop;
    }

    ACLMessage executeTask(const ACLMessage& /*accept*/) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ACLMessage result(ACLMessage::Performative::INFORM);
        result.setContent("livré:Paris;délai:2j;tarif:" + std::to_string(tarif_));
        return result;
    }

    void onEnd() override { this_agent->doDelete(); }
};

// ── Agents ────────────────────────────────────────────────────────────────────

struct DonneurCfg {
    std::string name;
    std::vector<AgentIdentifier> participants;
    char* winner; int* proposals; int* refuses; int* informs; char* content;
};

class DonneurAgent : public Agent {
    DonneurCfg cfg_;
public:
    explicit DonneurAgent(DonneurCfg cfg) : cfg_(std::move(cfg)) {}
    void setup() override {
        addBehaviour(new LivraisonInitiator(this, cfg_.name, cfg_.participants,
                                            cfg_.winner, cfg_.proposals,
                                            cfg_.refuses, cfg_.informs,
                                            cfg_.content));
    }
    void takeDown() override { acl_unlink(cfg_.name); }
};

class TransporteurAgent : public Agent {
    std::string name_; int tarif_; bool refuse_;
public:
    TransporteurAgent(std::string n, int t, bool r = false)
        : name_(std::move(n)), tarif_(t), refuse_(r) {}
    void setup()    override { addBehaviour(new TransporteurParticipant(this, name_, tarif_, refuse_)); }
    void takeDown() override { acl_unlink(name_); }
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
              << " FIPA Contract Net — deux scénarios\n"
              << "======================================================\n\n";

    init_shared();

    for (auto n : {"donneur","transp-a","transp-b","transp-c",
                   "donneur2","transp-d","transp-e"})
        acl_unlink(n);

    AgentCore::initAgentSystem();

    int ok = 0, fail = 0;

    // ── Scénario 1 : succès ───────────────────────────────────────────────────
    std::cout << "--- Scénario 1 : enchère (transp-a gagne) ---\n";
    {
        TransporteurAgent ta("transp-a", 70);
        TransporteurAgent tb("transp-b", 85);
        TransporteurAgent tc("transp-c", 0, true);
        ta.init(); tb.init(); tc.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        DonneurAgent donneur({
            "donneur",
            {AgentIdentifier{"transp-a"}, AgentIdentifier{"transp-b"}, AgentIdentifier{"transp-c"}},
            g_shared->winner, &g_shared->proposal_count,
            &g_shared->refuse_count, &g_shared->inform_received,
            g_shared->inform_content
        });
        donneur.init();

        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
    }

    std::cout << "  gagnant : " << g_shared->winner << "\n";
    run_check(std::string(g_shared->winner) == "transp-a",
              "gagnant = transp-a (moins cher 70€)",               ok, fail);
    run_check(g_shared->proposal_count == 2, "2 offres PROPOSE reçues",    ok, fail);
    run_check(g_shared->refuse_count   == 1, "1 REFUSE reçu (transp-c)",   ok, fail);
    run_check(g_shared->inform_received == 1, "INFORM de confirmation reçu", ok, fail);
    run_check(std::string(g_shared->inform_content).find("livré:Paris") != std::string::npos,
              "contenu INFORM contient \"livré:Paris\"",            ok, fail);

    // ── Scénario 2 : tous refusent ────────────────────────────────────────────
    std::cout << "\n--- Scénario 2 : tous les participants refusent ---\n";
    {
        TransporteurAgent td("transp-d", 0, true);
        TransporteurAgent te("transp-e", 0, true);
        td.init(); te.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        DonneurAgent donneur2({
            "donneur2",
            {AgentIdentifier{"transp-d"}, AgentIdentifier{"transp-e"}},
            g_shared->winner2, &g_shared->proposal_count2,
            &g_shared->refuse_count2, &g_shared->inform_received2,
            nullptr
        });
        donneur2.init();

        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
    }

    run_check(std::string(g_shared->winner2).empty(),
              "aucun gagnant (tous refusé)",                        ok, fail);
    run_check(g_shared->proposal_count2 == 0, "0 offres PROPOSE",          ok, fail);
    run_check(g_shared->refuse_count2   == 2, "2 REFUSE reçus",            ok, fail);
    run_check(g_shared->inform_received2 == 0, "aucun INFORM (pas d'ACCEPT envoyé)", ok, fail);

    // ── Bilan ─────────────────────────────────────────────────────────────────
    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << ok << " OK, " << fail << " FAIL\n\n";

    if (fail == 0) { std::cout << "[OK] Contract Net fonctionne\n"; return 0; }
    std::cout << "[FAIL] Erreurs détectées\n";
    return 1;
}
