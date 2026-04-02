/*
 * ContractNet.hpp — Protocole FIPA Contract Net (FIPA SC00029H)
 *
 * Deux classes Behaviour réutilisables :
 *
 *   ContractNetInitiator  — envoie un CFP, collecte les offres, sélectionne,
 *                           envoie accept/reject, attend le résultat
 *
 *   ContractNetParticipant — reçoit les CFP, propose ou refuse,
 *                            exécute si accepté, rapporte le résultat
 *
 * ── Flux FIPA ──────────────────────────────────────────────────────────────
 *
 *  Initiateur          Participants
 *      │                    │
 *      │── CFP ────────────►│   "qui peut faire X ?"
 *      │                    │
 *      │◄── PROPOSE ────────│   offre avec contenu libre
 *      │◄── REFUSE ─────────│   (optionnel)
 *      │
 *      │  [selectProposals() choisit le(s) gagnant(s)]
 *      │
 *      │── ACCEPT_PROPOSAL ►│   au(x) gagnant(s)
 *      │── REJECT_PROPOSAL ►│   aux autres
 *      │
 *      │◄── INFORM ─────────│   résultat de l'exécution
 *      │◄── FAILURE ────────│   (si la tâche a échoué)
 *
 * ── Usage ──────────────────────────────────────────────────────────────────
 *
 *  // Initiateur
 *  class MyInitiator : public ContractNetInitiator {
 *  public:
 *      MyInitiator(Agent* ag, std::string my_name, ACLMessage cfp,
 *                  std::vector<AgentIdentifier> participants)
 *          : ContractNetInitiator(ag, my_name, cfp, participants) {}
 *
 *      std::vector<std::string> selectProposals(
 *              const std::vector<ACLMessage>& proposals) override {
 *          // Retourner les noms des agents à accepter
 *          return { proposals[0].getSender().name }; // ex: prendre le premier
 *      }
 *
 *      void handleInform(const ACLMessage& msg) override {
 *          std::cout << "Résultat : " << msg.getContent() << "\n";
 *      }
 *  };
 *
 *  // Participant
 *  class MyParticipant : public ContractNetParticipant {
 *  public:
 *      MyParticipant(Agent* ag, std::string my_name)
 *          : ContractNetParticipant(ag, my_name) {}
 *
 *      ACLMessage prepareProposal(const ACLMessage& cfp) override {
 *          ACLMessage prop(ACLMessage::Performative::PROPOSE);
 *          prop.setContent("coût : 42");
 *          return prop;
 *      }
 *      ACLMessage executeTask(const ACLMessage& accept) override {
 *          ACLMessage result(ACLMessage::Performative::INFORM);
 *          result.setContent("tâche exécutée");
 *          return result;
 *      }
 *  };
 */

#ifndef GAGENT_PROTOCOLS_CONTRACTNET_HPP_
#define GAGENT_PROTOCOLS_CONTRACTNET_HPP_

#include <gagent/core/Behaviour.hpp>
#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/messaging/AgentIdentifier.hpp>

#include <string>
#include <vector>
#include <set>
#include <chrono>
#include <iostream>

namespace gagent {
namespace protocols {

// ── ContractNetInitiator ─────────────────────────────────────────────────────

class ContractNetInitiator : public Behaviour {
public:
    /**
     * @param ag                  pointeur vers l'agent
     * @param my_name             nom de l'initiateur (= nom de la queue /acl_<my_name>)
     * @param cfp                 message CFP à envoyer (content déjà rempli)
     * @param participants        liste des agents destinataires
     * @param proposal_timeout_ms délai max pour recevoir toutes les offres (ms)
     * @param result_timeout_ms   délai max pour recevoir le résultat (ms)
     */
    ContractNetInitiator(Agent*                          ag,
                         std::string                     my_name,
                         ACLMessage                      cfp,
                         std::vector<AgentIdentifier>    participants,
                         int proposal_timeout_ms = 5000,
                         int result_timeout_ms   = 10000)
        : Behaviour(ag)
        , my_name_(std::move(my_name))
        , cfp_(std::move(cfp))
        , participants_(std::move(participants))
        , proposal_timeout_ms_(proposal_timeout_ms)
        , result_timeout_ms_(result_timeout_ms)
    {}

    // ── À surcharger ─────────────────────────────────────────────────────────

    /**
     * Appelé quand toutes les offres sont reçues (ou timeout).
     * @param proposals  messages PROPOSE reçus
     * @return noms des agents dont l'offre est acceptée (vide = tout rejeter)
     */
    virtual std::vector<std::string> selectProposals(
            const std::vector<ACLMessage>& proposals) = 0;

    virtual void handleInform (const ACLMessage& /*msg*/) {}
    virtual void handleFailure(const ACLMessage& /*msg*/) {}
    virtual void handleRefuse (const ACLMessage& /*msg*/) {}

    // ── Behaviour interface ───────────────────────────────────────────────────

    void onStart() override {
        this_agent->transport().bind(my_name_);
    }

    void action() override
    {
        switch (state_) {

        case State::SEND_CFP: {
            conv_id_ = "cnp-" + my_name_ + "-" +
                std::to_string(std::chrono::steady_clock::now()
                               .time_since_epoch().count() % 100000);
            cfp_.setConversationId(conv_id_);
            cfp_.setSender(AgentIdentifier{my_name_});
            cfp_.setReplyWith("cfp-" + conv_id_);
            cfp_.setProtocol("fipa-contract-net");

            for (auto& p : participants_) {
                this_agent->transport().send(p.name, cfp_);
            }

            deadline_ = std::chrono::steady_clock::now()
                      + std::chrono::milliseconds(proposal_timeout_ms_);
            responded_ = 0;
            state_ = State::WAIT_PROPOSALS;
            break;
        }

        case State::WAIT_PROPOSALS: {
            auto now = std::chrono::steady_clock::now();
            if (now >= deadline_ || responded_ >= (int)participants_.size()) {
                state_ = State::HANDLE_PROPOSALS;
                break;
            }
            auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                            deadline_ - now).count();
            int  tmo  = static_cast<int>(std::min<long long>(500, left));

            auto opt = this_agent->transport().receive(my_name_, tmo);
            if (!opt) break;

            auto& msg = *opt;
            if (msg.getConversationId() != conv_id_) break; // message hors-protocole

            auto p = msg.getPerformative();
            if (p == ACLMessage::Performative::PROPOSE) {
                proposals_.push_back(msg);
                ++responded_;
            } else if (p == ACLMessage::Performative::REFUSE ||
                       p == ACLMessage::Performative::NOT_UNDERSTOOD) {
                handleRefuse(msg);
                ++responded_;
            }

            if (responded_ >= (int)participants_.size())
                state_ = State::HANDLE_PROPOSALS;
            break;
        }

        case State::HANDLE_PROPOSALS: {
            auto accepted_names = selectProposals(proposals_);
            std::set<std::string> accept_set(accepted_names.begin(), accepted_names.end());

            // Envoyer accept / reject aux participants qui ont proposé
            for (auto& prop : proposals_) {
                std::string pname = prop.getSender().name;
                if (accept_set.count(pname)) {
                    ACLMessage acc = prop.createReply(ACLMessage::Performative::ACCEPT_PROPOSAL);
                    acc.setSender(AgentIdentifier{my_name_});
                    this_agent->transport().send(pname, acc);
                    accepted_.push_back(pname);
                } else {
                    ACLMessage rej = prop.createReply(ACLMessage::Performative::REJECT_PROPOSAL);
                    rej.setSender(AgentIdentifier{my_name_});
                    this_agent->transport().send(pname, rej);
                }
            }

            if (accepted_.empty()) {
                state_ = State::DONE;
            } else {
                result_deadline_ = std::chrono::steady_clock::now()
                                 + std::chrono::milliseconds(result_timeout_ms_);
                results_pending_ = static_cast<int>(accepted_.size());
                state_ = State::WAIT_RESULTS;
            }
            break;
        }

        case State::WAIT_RESULTS: {
            auto now = std::chrono::steady_clock::now();
            if (now >= result_deadline_ || results_pending_ <= 0) {
                state_ = State::DONE;
                break;
            }
            auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                            result_deadline_ - now).count();
            int  tmo  = static_cast<int>(std::min<long long>(500, left));

            auto opt = this_agent->transport().receive(my_name_, tmo);
            if (!opt) break;

            auto& msg = *opt;
            if (msg.getConversationId() != conv_id_) break;

            auto p = msg.getPerformative();
            if (p == ACLMessage::Performative::INFORM) {
                handleInform(msg);
                --results_pending_;
            } else if (p == ACLMessage::Performative::FAILURE) {
                handleFailure(msg);
                --results_pending_;
            }

            if (results_pending_ <= 0)
                state_ = State::DONE;
            break;
        }

        case State::DONE:
            done_ = true;
            break;
        }
    }

    bool done() override { return done_; }

private:
    enum class State { SEND_CFP, WAIT_PROPOSALS, HANDLE_PROPOSALS, WAIT_RESULTS, DONE };

    std::string                  my_name_;
    ACLMessage                   cfp_;
    std::vector<AgentIdentifier> participants_;
    int                          proposal_timeout_ms_;
    int                          result_timeout_ms_;

    State       state_    = State::SEND_CFP;
    bool        done_     = false;
    std::string conv_id_;

    std::vector<ACLMessage> proposals_;
    std::vector<std::string> accepted_;
    int  responded_       = 0;
    int  results_pending_ = 0;

    std::chrono::steady_clock::time_point deadline_;
    std::chrono::steady_clock::time_point result_deadline_;
};


// ── ContractNetParticipant ───────────────────────────────────────────────────

class ContractNetParticipant : public Behaviour {
public:
    /**
     * @param ag          pointeur vers l'agent
     * @param my_name     nom du participant (= nom de la queue /acl_<my_name>)
     * @param timeout_ms  délai d'attente max pour recevoir le CFP (ms)
     */
    ContractNetParticipant(Agent*       ag,
                           std::string  my_name,
                           int          timeout_ms = 8000)
        : Behaviour(ag)
        , my_name_(std::move(my_name))
        , timeout_ms_(timeout_ms)
    {}

    // ── À surcharger ─────────────────────────────────────────────────────────

    /**
     * Évaluer le CFP et retourner une réponse.
     * @param cfp  message CFP reçu
     * @return ACLMessage avec performatif PROPOSE (content = offre)
     *         ou REFUSE (content = raison du refus)
     */
    virtual ACLMessage prepareProposal(const ACLMessage& cfp) = 0;

    /**
     * Exécuter la tâche assignée par l'ACCEPT_PROPOSAL.
     * @param accept  message ACCEPT_PROPOSAL reçu
     * @return ACLMessage avec performatif INFORM (content = résultat)
     *         ou FAILURE (content = raison de l'échec)
     */
    virtual ACLMessage executeTask(const ACLMessage& accept) = 0;

    // ── Behaviour interface ───────────────────────────────────────────────────

    void onStart() override {
        // Pré-bind avant que l'initiateur envoie le CFP,
        // pour éviter de perdre le message (ZMQ PUSH drop si pas de peer)
        this_agent->transport().bind(my_name_);
    }

    void action() override
    {
        switch (state_) {

        case State::WAIT_CFP: {
            auto opt = this_agent->transport().receive(my_name_, timeout_ms_);
            if (!opt) { done_ = true; break; }  // timeout global → terminé

            auto& msg = *opt;
            if (msg.getPerformative() == ACLMessage::Performative::CFP) {
                ACLMessage resp = prepareProposal(msg);
                resp.setSender(AgentIdentifier{my_name_});
                resp.setConversationId(msg.getConversationId());
                resp.setInReplyTo(msg.getReplyWith());
                this_agent->transport().send(msg.getSender().name, resp);

                if (resp.getPerformative() == ACLMessage::Performative::REFUSE) {
                    done_ = true;   // refus → protocole terminé pour ce participant
                } else {
                    conv_id_ = msg.getConversationId();
                    initiator_ = msg.getSender().name;
                    state_ = State::WAIT_DECISION;
                }
            }
            break;
        }

        case State::WAIT_DECISION: {
            auto opt = this_agent->transport().receive(my_name_, timeout_ms_);
            if (!opt) { done_ = true; break; }  // timeout → abandon

            auto& msg = *opt;
            if (msg.getConversationId() != conv_id_) break;

            auto p = msg.getPerformative();
            if (p == ACLMessage::Performative::ACCEPT_PROPOSAL) {
                ACLMessage result = executeTask(msg);
                result.setSender(AgentIdentifier{my_name_});
                result.setConversationId(conv_id_);
                result.setInReplyTo(msg.getReplyWith());
                this_agent->transport().send(initiator_, result);
                done_ = true;
            } else if (p == ACLMessage::Performative::REJECT_PROPOSAL) {
                done_ = true;
            }
            break;
        }
        }
    }

    bool done() override { return done_; }

private:
    enum class State { WAIT_CFP, WAIT_DECISION };

    std::string my_name_;
    int         timeout_ms_;
    State       state_    = State::WAIT_CFP;
    bool        done_     = false;
    std::string conv_id_;
    std::string initiator_;
};

} // namespace protocols
} // namespace gagent

#endif /* GAGENT_PROTOCOLS_CONTRACTNET_HPP_ */
