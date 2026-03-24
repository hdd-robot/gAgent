/*
 * Request.hpp — Protocole FIPA Request (FIPA SC00026H)
 *
 * Deux classes Behaviour réutilisables :
 *
 *   RequestInitiator  — envoie une REQUEST, attend INFORM/REFUSE/FAILURE
 *
 *   RequestParticipant — reçoit les REQUEST, retourne une réponse
 *
 * ── Flux FIPA ──────────────────────────────────────────────────────────────
 *
 *  Initiateur          Participant
 *      │                   │
 *      │── REQUEST ────────►│
 *      │                   │
 *      │◄── AGREE ──────────│   (optionnel, si traitement long)
 *      │                   │
 *      │◄── INFORM ─────────│   succès
 *      │   (ou REFUSE)      │   refus immédiat
 *      │   (ou FAILURE)     │   échec après AGREE
 *      │   (ou NOT_UNDERSTOOD)│
 *
 * ── Usage ──────────────────────────────────────────────────────────────────
 *
 *  // Initiateur
 *  class MyRequester : public RequestInitiator {
 *  public:
 *      MyRequester(Agent* ag, const std::string& my_name,
 *                  const std::string& target)
 *          : RequestInitiator(ag, my_name, target, "compute-something") {}
 *
 *      void handleInform(const ACLMessage& msg) override {
 *          std::cout << "Résultat : " << msg.getContent() << "\n";
 *      }
 *      void handleRefuse(const ACLMessage& msg) override {
 *          std::cout << "Refusé : " << msg.getContent() << "\n";
 *      }
 *  };
 *
 *  // Participant
 *  class MyResponder : public RequestParticipant {
 *  public:
 *      MyResponder(Agent* ag, const std::string& my_name)
 *          : RequestParticipant(ag, my_name) {}
 *
 *      ACLMessage handleRequest(const ACLMessage& req) override {
 *          ACLMessage reply = req.createReply(ACLMessage::Performative::INFORM);
 *          reply.setContent("result : 42");
 *          return reply;
 *      }
 *  };
 *
 *  // Dans setup()
 *  addBehaviour(new MyRequester(this, "alice", "bob"));
 *  addBehaviour(new MyResponder(this, "bob"));
 */

#ifndef GAGENT_REQUEST_HPP_
#define GAGENT_REQUEST_HPP_

#include <gagent/core/Behaviour.hpp>
#include <gagent/messaging/AclMQ.hpp>
#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/messaging/AgentIdentifier.hpp>
#include <string>

namespace gagent {
namespace protocols {

using messaging::acl_send;
using messaging::acl_receive;
using messaging::acl_bind;

// ─────────────────────────────────────────────────────────────────────────────
// RequestInitiator
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Côté initiateur du protocole FIPA Request.
 *
 * Envoie une REQUEST à un agent cible et attend sa réponse.
 * Termine après réception d'une réponse (INFORM, REFUSE, FAILURE,
 * NOT_UNDERSTOOD) ou après timeout.
 */
class RequestInitiator : public Behaviour {
public:
    /**
     * @param ag          Agent propriétaire
     * @param my_name     Nom de cet agent (pour recevoir la réponse)
     * @param target      Nom de l'agent cible
     * @param content     Contenu de la requête
     * @param ontology    Ontologie (optionnel)
     * @param timeout_ms  Timeout d'attente de la réponse (défaut : 10 s)
     */
    RequestInitiator(Agent* ag,
                     const std::string& my_name,
                     const std::string& target,
                     const std::string& content,
                     const std::string& ontology  = "",
                     int                timeout_ms = 10000)
        : Behaviour(ag)
        , my_name_(my_name)
        , target_(target)
        , content_(content)
        , ontology_(ontology)
        , timeout_ms_(timeout_ms)
    {
        conv_id_ = "req-" + my_name_ + "-" + std::to_string(::getpid());
    }

    void onStart() override {
        // Pré-bind le socket PULL avant d'envoyer la requête,
        // pour ne pas perdre la réponse si le serveur répond très vite
        acl_bind(my_name_);
    }

    void action() override {
        switch (state_) {
        case SEND_REQUEST: {
            ACLMessage req(ACLMessage::Performative::REQUEST);
            req.setSender(AgentIdentifier{my_name_});
            req.addReceiver(AgentIdentifier{target_});
            req.setContent(content_);
            req.setConversationId(conv_id_);
            if (!ontology_.empty()) req.setOntology(ontology_);
            req.setProtocol("fipa-request");

            acl_send(target_, req);
            state_ = WAIT_RESPONSE;
            break;
        }

        case WAIT_RESPONSE: {
            auto opt = acl_receive(my_name_, timeout_ms_);
            if (!opt) {
                handleTimeout();
                done_ = true;
                break;
            }
            const ACLMessage& msg = *opt;
            if (msg.getConversationId() != conv_id_) break; // pas pour nous

            switch (msg.getPerformative()) {
            case ACLMessage::Performative::AGREE:
                handleAgree(msg);
                // reste en WAIT_RESPONSE pour attendre INFORM/FAILURE
                break;
            case ACLMessage::Performative::INFORM:
                handleInform(msg);
                done_ = true;
                break;
            case ACLMessage::Performative::REFUSE:
                handleRefuse(msg);
                done_ = true;
                break;
            case ACLMessage::Performative::FAILURE:
                handleFailure(msg);
                done_ = true;
                break;
            case ACLMessage::Performative::NOT_UNDERSTOOD:
                handleNotUnderstood(msg);
                done_ = true;
                break;
            default:
                break;
            }
            break;
        }
        }
    }

    bool done() override { return done_; }

    // ── Callbacks à surcharger ──────────────────────────────────────────────

    /** Appelé quand la requête est acceptée (traitement en cours). */
    virtual void handleAgree(const ACLMessage&) {}

    /** Appelé quand la requête aboutit. */
    virtual void handleInform(const ACLMessage&) {}

    /** Appelé quand la requête est refusée. */
    virtual void handleRefuse(const ACLMessage&) {}

    /** Appelé quand la requête a échoué après acceptation. */
    virtual void handleFailure(const ACLMessage&) {}

    /** Appelé quand la requête n'est pas comprise. */
    virtual void handleNotUnderstood(const ACLMessage&) {}

    /** Appelé en cas de timeout (pas de réponse). */
    virtual void handleTimeout() {}

protected:
    std::string my_name_;
    std::string target_;
    std::string content_;
    std::string ontology_;
    std::string conv_id_;
    int         timeout_ms_;

private:
    enum State { SEND_REQUEST, WAIT_RESPONSE };
    State state_ = SEND_REQUEST;
    bool  done_  = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// RequestParticipant
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Côté participant du protocole FIPA Request.
 *
 * Écoute les REQUEST entrantes, appelle handleRequest() pour construire
 * la réponse, et la renvoie à l'initiateur.
 *
 * Tourne indéfiniment (done() retourne toujours false).
 */
class RequestParticipant : public Behaviour {
public:
    /**
     * @param ag          Agent propriétaire
     * @param my_name     Nom de cet agent (queue d'écoute)
     * @param tick_ms     Timeout de polling (défaut : 500 ms)
     */
    RequestParticipant(Agent* ag,
                       const std::string& my_name,
                       int tick_ms = 500)
        : Behaviour(ag)
        , my_name_(my_name)
        , tick_ms_(tick_ms)
    {}

    void action() override {
        auto opt = acl_receive(my_name_, tick_ms_);
        if (!opt) return;
        const ACLMessage& msg = *opt;

        if (msg.getPerformative() != ACLMessage::Performative::REQUEST) return;

        ACLMessage response = handleRequest(msg);
        // S'assurer que la réponse est bien adressée à l'expéditeur
        if (response.getReceivers().empty()) {
            response.addReceiver(msg.getSender());
        }
        if (response.getSender().name.empty()) {
            response.setSender(AgentIdentifier{my_name_});
        }
        if (response.getConversationId().empty()) {
            response.setConversationId(msg.getConversationId());
        }

        const std::string& dest = response.getReceivers()[0].name;
        acl_send(dest, response);
    }

    bool done() override { return false; }

    // ── Callback à surcharger ───────────────────────────────────────────────

    /**
     * Traite la requête et retourne la réponse.
     *
     * Par défaut retourne INFORM vide. Surcharger pour implémenter la logique
     * métier. Retourner REFUSE ou FAILURE si nécessaire.
     *
     * Exemple :
     *   ACLMessage handleRequest(const ACLMessage& req) override {
     *       auto reply = req.createReply(ACLMessage::Performative::INFORM);
     *       reply.setContent("done");
     *       return reply;
     *   }
     */
    virtual ACLMessage handleRequest(const ACLMessage& req) {
        ACLMessage reply = req.createReply(ACLMessage::Performative::INFORM);
        reply.setSender(AgentIdentifier{my_name_});
        return reply;
    }

protected:
    std::string my_name_;
    int         tick_ms_;
};

} // namespace protocols
} // namespace gagent

#endif /* GAGENT_REQUEST_HPP_ */
