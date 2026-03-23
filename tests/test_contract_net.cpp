/*
 * test_contract_net.cpp — Test d'intégration du protocole FIPA Contract Net
 *
 * Scénario : enchère de livraison
 *
 *   Donneur (initiateur) demande : "livrer colis à Paris, budget 100€"
 *   3 transporteurs (participants) répondent avec leurs tarifs
 *   Le donneur accepte le moins cher
 *   Le transporteur sélectionné confirme la livraison
 *
 * Flux observé :
 *   Donneur  → CFP        → Transporteur{A,B,C}
 *   Transp.A ← PROPOSE 70 → Donneur
 *   Transp.B ← PROPOSE 85 → Donneur
 *   Transp.C ← REFUSE      → Donneur   (surchargé)
 *   Donneur  → ACCEPT      → Transp.A  (moins cher)
 *   Donneur  → REJECT      → Transp.B
 *   Transp.A → INFORM      → Donneur   (livraison confirmée)
 */

#include <gagent/core/AgentCore.hpp>
#include <gagent/env/Environnement.hpp>
#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/protocols/ContractNet.hpp>
#include <gagent/messaging/AclMQ.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace gagent;
using namespace gagent::protocols;
using namespace gagent::messaging;

// ── Environnement minimal ─────────────────────────────────────────────────────

class DummyEnv : public Environnement {
public:
    void init_env()      override {}
    void link_attribut() override {}
    void event_loop()    override {}
};

// ── Comportement du donneur ───────────────────────────────────────────────────

class LivraisonInitiator : public ContractNetInitiator {
    bool success_ = false;
public:
    LivraisonInitiator(Agent* ag,
                       const std::string& my_name,
                       const std::vector<AgentIdentifier>& transporteurs)
        : ContractNetInitiator(
            ag,
            my_name,
            []() {
                ACLMessage cfp(ACLMessage::Performative::CFP);
                cfp.setContent("livraison:Paris;poids:5kg;budget:100");
                cfp.setOntology("logistics");
                cfp.setLanguage("fipa-sl");
                return cfp;
            }(),
            transporteurs,
            4000,   // 4 s pour les offres
            6000)   // 6 s pour le résultat
    {}

    // Sélectionner le transporteur avec le tarif le plus bas
    std::vector<std::string> selectProposals(
            const std::vector<ACLMessage>& proposals) override
    {
        std::cout << "\n[Donneur] " << proposals.size()
                  << " offre(s) reçue(s) :\n";

        std::string best_name;
        int         best_price = INT_MAX;

        for (auto& p : proposals) {
            // Le contenu est "tarif:<N>"
            int price = 999;
            auto& c = p.getContent();
            auto pos = c.find("tarif:");
            if (pos != std::string::npos)
                price = std::stoi(c.substr(pos + 6));

            std::cout << "  " << p.getSender().name
                      << " → " << price << "€\n";

            if (price < best_price) {
                best_price = price;
                best_name  = p.getSender().name;
            }
        }

        if (best_name.empty()) {
            std::cout << "[Donneur] aucune offre retenue\n";
            return {};
        }

        std::cout << "[Donneur] accepté : " << best_name
                  << " (" << best_price << "€)\n";
        return { best_name };
    }

    void handleRefuse(const ACLMessage& msg) override {
        std::cout << "[Donneur] " << msg.getSender().name
                  << " a refusé : " << msg.getContent() << "\n" << std::flush;
    }

    void handleInform(const ACLMessage& msg) override {
        std::cout << "[Donneur] résultat de "
                  << msg.getSender().name << " : "
                  << msg.getContent() << "\n" << std::flush;
        success_ = true;
    }

    void handleFailure(const ACLMessage& msg) override {
        std::cout << "[Donneur] échec de "
                  << msg.getSender().name << " : "
                  << msg.getContent() << "\n" << std::flush;
    }

    bool success() const { return success_; }

    void onEnd() override {
        std::cout << "[Donneur] protocole terminé\n";
        this_agent->doDelete();
    }
};

// ── Comportement des transporteurs ───────────────────────────────────────────

class TransporteurParticipant : public ContractNetParticipant {
    std::string name_;
    int         tarif_;
    bool        refuse_;  // true = surchargé, refuse le CFP
public:
    TransporteurParticipant(Agent* ag, const std::string& name,
                            int tarif, bool refuse = false)
        : ContractNetParticipant(ag, name, 5000)
        , name_(name), tarif_(tarif), refuse_(refuse)
    {}

    ACLMessage prepareProposal(const ACLMessage& cfp) override {
        std::cout << "[" << name_ << "] CFP reçu : "
                  << cfp.getContent() << "\n" << std::flush;

        if (refuse_) {
            std::cout << "[" << name_ << "] refuse (surchargé)\n";
            ACLMessage r(ACLMessage::Performative::REFUSE);
            r.setContent("surchargé");
            return r;
        }

        std::cout << "[" << name_ << "] propose " << tarif_ << "€\n";
        ACLMessage prop(ACLMessage::Performative::PROPOSE);
        prop.setContent("tarif:" + std::to_string(tarif_));
        return prop;
    }

    ACLMessage executeTask(const ACLMessage& /*accept*/) override {
        std::cout << "[" << name_ << "] livraison en cours...\n";
        // Simulation d'une tâche
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "[" << name_ << "] livraison confirmée\n";

        ACLMessage result(ACLMessage::Performative::INFORM);
        result.setContent("livré:Paris;délai:2j;tarif:" + std::to_string(tarif_));
        return result;
    }

    void onEnd() override {
        this_agent->doDelete();
    }
};

// ── Agents ────────────────────────────────────────────────────────────────────

class DonneurAgent : public Agent {
    std::vector<AgentIdentifier> transporteurs_;
public:
    explicit DonneurAgent(std::vector<AgentIdentifier> t)
        : transporteurs_(std::move(t)) {}

    void setup() override {
        std::cout << "[Donneur] démarrage\n";
        addBehaviour(new LivraisonInitiator(this, "donneur", transporteurs_));
    }
    void takeDown() override { acl_unlink("donneur"); }
};

class TransporteurAgent : public Agent {
    std::string name_;
    int         tarif_;
    bool        refuse_;
public:
    TransporteurAgent(std::string name, int tarif, bool refuse = false)
        : name_(std::move(name)), tarif_(tarif), refuse_(refuse) {}

    void setup() override {
        std::cout << "[" << name_ << "] démarrage (tarif " << tarif_ << "€)\n" << std::flush;
        addBehaviour(new TransporteurParticipant(this, name_, tarif_, refuse_));
    }

    void takeDown() override { acl_unlink(name_); }
};

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "======================================================\n"
              << " FIPA Contract Net — enchère de livraison\n"
              << "======================================================\n\n";

    // Nettoyage des queues résiduelles
    acl_unlink("donneur");
    acl_unlink("transp-a");
    acl_unlink("transp-b");
    acl_unlink("transp-c");

    AgentCore::initAgentSystem();

    DummyEnv env;
    AgentCore::initEnvironnementSystem(env);

    // Participants (se lancent avant l'initiateur pour que leurs queues soient prêtes)
    TransporteurAgent ta("transp-a", 70);          // propose 70€
    TransporteurAgent tb("transp-b", 85);          // propose 85€
    TransporteurAgent tc("transp-c", 0, true);     // refuse (surchargé)

    ta.init();
    tb.init();
    tc.init();

    // Petit délai pour que les participants soient prêts
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Initiateur
    DonneurAgent donneur({
        AgentIdentifier{"transp-a"},
        AgentIdentifier{"transp-b"},
        AgentIdentifier{"transp-c"}
    });
    donneur.init();

    // Attente de la fin des 4 agents
    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();

    std::cout << "\n=== Contract Net terminé ===\n";
    return 0;
}
