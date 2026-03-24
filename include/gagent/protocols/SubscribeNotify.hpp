/*
 * SubscribeNotify.hpp — Protocole FIPA Subscribe-Notify (FIPA SC00035H)
 *
 * Deux classes Behaviour réutilisables :
 *
 *   SubscribeInitiator  — s'abonne à un publisher, reçoit les notifications
 *
 *   SubscribeParticipant — reçoit les abonnements, notifie ses subscribers
 *
 * ── Flux FIPA ──────────────────────────────────────────────────────────────
 *
 *  Subscriber          Publisher
 *      │                   │
 *      │── SUBSCRIBE ──────►│   "notifie-moi quand X change"
 *      │                   │
 *      │◄── AGREE ──────────│   accepté
 *      │   (ou REFUSE)      │   refusé → done
 *      │                   │
 *      │             [événement]
 *      │◄── INFORM ─────────│   notification (répétée)
 *      │◄── INFORM ─────────│
 *      │                   │
 *      │── CANCEL ──────────►│   se désabonner (optionnel)
 *      │◄── INFORM ─────────│   confirmation finale
 *
 * ── Usage ──────────────────────────────────────────────────────────────────
 *
 *  // Subscriber
 *  class MonSubscriber : public SubscribeInitiator {
 *  public:
 *      MonSubscriber(Agent* ag, const std::string& my_name,
 *                   const std::string& publisher)
 *          : SubscribeInitiator(ag, my_name, publisher,
 *                               "temperature",   // sujet
 *                               "sensors")       // ontologie
 *      {}
 *
 *      void handleNotify(const ACLMessage& msg) override {
 *          std::cout << "Notification : " << msg.getContent() << "\n";
 *      }
 *      void handleRefuse(const ACLMessage& msg) override {
 *          std::cout << "Refusé : " << msg.getContent() << "\n";
 *      }
 *  };
 *
 *  // Publisher
 *  class MonPublisher : public SubscribeParticipant {
 *  public:
 *      MonPublisher(Agent* ag, const std::string& my_name)
 *          : SubscribeParticipant(ag, my_name) {}
 *
 *      bool handleSubscribe(const ACLMessage& sub) override {
 *          return true;  // accepter tous les abonnements
 *      }
 *
 *      // Appeler notify("nouvelle valeur") quand l'état change
 *  };
 */

#ifndef GAGENT_PROTOCOLS_SUBSCRIBENOTIFY_HPP_
#define GAGENT_PROTOCOLS_SUBSCRIBENOTIFY_HPP_

#include <gagent/core/Behaviour.hpp>
#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/messaging/AgentIdentifier.hpp>
#include <gagent/messaging/AclMQ.hpp>

#include <string>
#include <vector>
#include <chrono>
#include <iostream>

namespace gagent {
namespace protocols {

using messaging::acl_send;
using messaging::acl_receive;
using messaging::acl_unlink;
using messaging::acl_bind;

// ── SubscribeInitiator ────────────────────────────────────────────────────────

class SubscribeInitiator : public Behaviour {
public:
    /**
     * @param ag            pointeur vers l'agent
     * @param my_name       nom de la queue ACL (/acl_<my_name>)
     * @param publisher     nom de l'agent publisher à qui s'abonner
     * @param topic         contenu du SUBSCRIBE (sujet / filtre)
     * @param ontology      ontologie (optionnel)
     * @param agree_timeout délai max pour recevoir le AGREE (ms)
     */
    SubscribeInitiator(Agent*             ag,
                       const std::string& my_name,
                       const std::string& publisher,
                       const std::string& topic          = "",
                       const std::string& ontology       = "",
                       int                agree_timeout  = 5000)
        : Behaviour(ag)
        , my_name_(my_name)
        , publisher_(publisher)
        , topic_(topic)
        , ontology_(ontology)
        , agree_timeout_(agree_timeout)
    {}

    // ── À surcharger ─────────────────────────────────────────────────────────

    /** Appelé à chaque INFORM reçu du publisher. */
    virtual void handleNotify(const ACLMessage& msg) {}

    /** Appelé si le publisher refuse l'abonnement. */
    virtual void handleRefuse(const ACLMessage& msg) {}

    /**
     * Appelé périodiquement (tick).
     * Retourner true pour envoyer CANCEL et se désabonner.
     */
    virtual bool shouldCancel() { return false; }

    // ── Behaviour interface ───────────────────────────────────────────────────

    void onStart() override {
        acl_bind(my_name_);
    }

    void action() override
    {
        switch (state_) {

        case State::SEND_SUBSCRIBE: {
            conv_id_ = "snp-" + my_name_ + "-" +
                std::to_string(std::chrono::steady_clock::now()
                               .time_since_epoch().count() % 100000);

            ACLMessage sub(ACLMessage::Performative::SUBSCRIBE);
            sub.setSender(AgentIdentifier{my_name_});
            sub.setContent(topic_);
            if (!ontology_.empty()) sub.setOntology(ontology_);
            sub.setConversationId(conv_id_);
            sub.setReplyWith("sub-" + conv_id_);
            sub.setProtocol("fipa-subscribe");
            acl_send(publisher_, sub);

            deadline_ = std::chrono::steady_clock::now()
                      + std::chrono::milliseconds(agree_timeout_);
            state_ = State::WAIT_AGREE;
            break;
        }

        case State::WAIT_AGREE: {
            auto now = std::chrono::steady_clock::now();
            if (now >= deadline_) {
                std::cerr << "[SubscribeInitiator] timeout AGREE\n";
                done_ = true;
                break;
            }
            auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                            deadline_ - now).count();
            int tmo = static_cast<int>(std::min<long long>(500, left));

            auto opt = acl_receive(my_name_, tmo);
            if (!opt) break;

            auto& msg = *opt;
            if (msg.getConversationId() != conv_id_) break;

            auto p = msg.getPerformative();
            if (p == ACLMessage::Performative::AGREE) {
                state_ = State::WAIT_NOTIFY;
            } else if (p == ACLMessage::Performative::REFUSE ||
                       p == ACLMessage::Performative::NOT_UNDERSTOOD) {
                handleRefuse(msg);
                done_ = true;
            }
            break;
        }

        case State::WAIT_NOTIFY: {
            if (shouldCancel()) {
                ACLMessage cancel(ACLMessage::Performative::CANCEL);
                cancel.setSender(AgentIdentifier{my_name_});
                cancel.setConversationId(conv_id_);
                cancel.setProtocol("fipa-subscribe");
                acl_send(publisher_, cancel);
                state_ = State::WAIT_CANCEL_ACK;
                break;
            }

            auto opt = acl_receive(my_name_, 300);
            if (!opt) break;

            auto& msg = *opt;
            if (msg.getConversationId() != conv_id_) break;

            if (msg.getPerformative() == ACLMessage::Performative::INFORM)
                handleNotify(msg);
            break;
        }

        case State::WAIT_CANCEL_ACK: {
            auto opt = acl_receive(my_name_, 2000);
            if (opt && opt->getConversationId() == conv_id_)
                handleNotify(*opt);  // dernier INFORM éventuel
            done_ = true;
            break;
        }
        }
    }

    bool done() override { return done_; }

private:
    enum class State { SEND_SUBSCRIBE, WAIT_AGREE, WAIT_NOTIFY, WAIT_CANCEL_ACK };

    std::string my_name_;
    std::string publisher_;
    std::string topic_;
    std::string ontology_;
    int         agree_timeout_;

    State  state_   = State::SEND_SUBSCRIBE;
    bool   done_    = false;
    std::string conv_id_;
    std::chrono::steady_clock::time_point deadline_;
};


// ── SubscribeParticipant ──────────────────────────────────────────────────────

class SubscribeParticipant : public Behaviour {
public:
    /**
     * @param ag         pointeur vers l'agent
     * @param my_name    nom de la queue ACL (/acl_<my_name>)
     * @param poll_ms    intervalle de poll pour les nouveaux abonnements (ms)
     */
    SubscribeParticipant(Agent*             ag,
                         const std::string& my_name,
                         int                poll_ms = 200)
        : Behaviour(ag)
        , my_name_(my_name)
        , poll_ms_(poll_ms)
    {}

    // ── À surcharger ─────────────────────────────────────────────────────────

    /**
     * Appelé quand un SUBSCRIBE arrive.
     * @return true pour accepter (AGREE), false pour refuser (REFUSE)
     */
    virtual bool handleSubscribe(const ACLMessage& msg) { return true; }

    /**
     * Appelé quand un subscriber envoie CANCEL.
     * Permet de faire un dernier INFORM avant de supprimer l'abonnement.
     * @return contenu du dernier INFORM (vide = pas de dernier message)
     */
    virtual std::string handleCancel(const std::string& subscriber) { return ""; }

    // ── Interface publique ────────────────────────────────────────────────────

    /**
     * Notifier tous les subscribers actifs.
     * À appeler depuis onTick() ou depuis un autre behaviour quand l'état change.
     */
    void notify(const std::string& content,
                const std::string& ontology = "")
    {
        std::vector<std::string> dead;
        for (auto& [name, sub] : subscribers_) {
            ACLMessage inf(ACLMessage::Performative::INFORM);
            inf.setSender(AgentIdentifier{my_name_});
            inf.setContent(content);
            inf.setConversationId(sub.conv_id);
            inf.setProtocol("fipa-subscribe");
            if (!ontology.empty()) inf.setOntology(ontology);
            acl_send(name, inf);
        }
    }

    /** Notifier un seul subscriber. */
    void notifyOne(const std::string& subscriber,
                   const std::string& content,
                   const std::string& ontology = "")
    {
        auto it = subscribers_.find(subscriber);
        if (it == subscribers_.end()) return;
        ACLMessage inf(ACLMessage::Performative::INFORM);
        inf.setSender(AgentIdentifier{my_name_});
        inf.setContent(content);
        inf.setConversationId(it->second.conv_id);
        inf.setProtocol("fipa-subscribe");
        if (!ontology.empty()) inf.setOntology(ontology);
        acl_send(subscriber, inf);
    }

    /** Nombre de subscribers actifs. */
    int subscriberCount() const { return (int)subscribers_.size(); }

    // ── Behaviour interface ───────────────────────────────────────────────────

    void action() override
    {
        auto opt = acl_receive(my_name_, poll_ms_);
        if (!opt) return;

        auto& msg = *opt;
        auto  p   = msg.getPerformative();
        std::string sender = msg.getSender().name;

        if (p == ACLMessage::Performative::SUBSCRIBE) {
            bool accepted = handleSubscribe(msg);
            if (accepted) {
                ACLMessage agree = msg.createReply(ACLMessage::Performative::AGREE);
                agree.setSender(AgentIdentifier{my_name_});
                agree.setProtocol("fipa-subscribe");
                acl_send(sender, agree);
                subscribers_[sender] = { msg.getConversationId() };
            } else {
                ACLMessage refuse = msg.createReply(ACLMessage::Performative::REFUSE);
                refuse.setSender(AgentIdentifier{my_name_});
                refuse.setProtocol("fipa-subscribe");
                acl_send(sender, refuse);
            }

        } else if (p == ACLMessage::Performative::CANCEL) {
            std::string last = handleCancel(sender);
            if (!last.empty()) {
                ACLMessage inf(ACLMessage::Performative::INFORM);
                inf.setSender(AgentIdentifier{my_name_});
                inf.setContent(last);
                inf.setConversationId(msg.getConversationId());
                inf.setProtocol("fipa-subscribe");
                acl_send(sender, inf);
            }
            subscribers_.erase(sender);
        }
    }

    bool done() override { return false; }  // publisher tourne indéfiniment

private:
    struct Subscription { std::string conv_id; };

    std::string my_name_;
    int         poll_ms_;
    std::map<std::string, Subscription> subscribers_;
};

} // namespace protocols
} // namespace gagent

#endif /* GAGENT_PROTOCOLS_SUBSCRIBENOTIFY_HPP_ */
