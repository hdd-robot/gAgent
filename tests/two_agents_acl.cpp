/*
 * two_agents_acl.cpp
 *
 * Démonstration : deux agents échangent des messages FIPA ACL
 *
 *  Alice → REQUEST  "Quelle heure est-il ?" → Bob
 *  Bob   → INFORM   "Il est HH:MM:SS"       → Alice
 *
 * Chaque agent tourne dans son propre processus (fork).
 * La messagerie ACL passe par des POSIX MQ dédiées /acl_alice et /acl_bob,
 * distinctes des queues internes de contrôle de l'agent.
 */

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstring>
#include <mqueue.h>

#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
#include <gagent/env/Environnement.hpp>
#include <gagent/messaging/ACLMessage.hpp>

using namespace gagent;

// ── Helpers de messagerie ACL ─────────────────────────────────────────────────

static constexpr int ACL_MSG_SIZE = 1024;
static constexpr int ACL_MSG_MAX  = 10;

static std::string acl_qname(const std::string& agent_name)
{
    return "/acl_" + agent_name;
}

// Envoie un ACLMessage dans la queue de l'agent cible (crée la queue si besoin)
static bool acl_send(const std::string& to, const ACLMessage& msg)
{
    std::string raw = msg.toString();
    if ((int)raw.size() >= ACL_MSG_SIZE) {
        std::cerr << "[acl_send] message trop grand (" << raw.size() << " octets)\n";
        return false;
    }
    struct mq_attr attr{};
    attr.mq_maxmsg  = ACL_MSG_MAX;
    attr.mq_msgsize = ACL_MSG_SIZE;

    mqd_t mq = mq_open(acl_qname(to).c_str(), O_WRONLY | O_CREAT, 0666, &attr);
    if (mq == (mqd_t)-1) { perror("acl_send mq_open"); return false; }
    int r = mq_send(mq, raw.c_str(), raw.size(), 0);
    mq_close(mq);
    return r == 0;
}

// Attend un ACLMessage sur la queue de cet agent (bloquant avec timeout en ms)
static std::optional<ACLMessage> acl_receive(const std::string& my_name, int timeout_ms = 5000)
{
    struct mq_attr attr{};
    attr.mq_maxmsg  = ACL_MSG_MAX;
    attr.mq_msgsize = ACL_MSG_SIZE;

    mqd_t mq = mq_open(acl_qname(my_name).c_str(), O_RDONLY | O_CREAT, 0666, &attr);
    if (mq == (mqd_t)-1) { perror("acl_receive mq_open"); return std::nullopt; }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000LL;
    if (ts.tv_nsec >= 1000000000LL) { ts.tv_sec++; ts.tv_nsec -= 1000000000LL; }

    std::vector<char> buf(ACL_MSG_SIZE);
    ssize_t n = mq_timedreceive(mq, buf.data(), ACL_MSG_SIZE, nullptr, &ts);
    mq_close(mq);

    if (n <= 0) return std::nullopt;
    return ACLMessage::parse(std::string(buf.data(), static_cast<size_t>(n)));
}

// ── Behaviour d'Alice : envoie N REQUEST à Bob ────────────────────────────────

class AliceSendBehaviour : public Behaviour {
    std::string target_;
    int count_ = 0;
    static constexpr int MAX = 4;
public:
    AliceSendBehaviour(Agent* ag, const std::string& target)
        : Behaviour(ag), target_(target) {}

    void action() override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        ++count_;
        ACLMessage req(ACLMessage::Performative::REQUEST);
        req.setSender    (AgentIdentifier{"alice"});
        req.addReceiver  (AgentIdentifier{target_});
        req.setContent   ("Quelle heure est-il ? (msg " +
                          std::to_string(count_) + "/" + std::to_string(MAX) + ")");
        req.setLanguage  ("fipa-sl");
        req.setOntology  ("time-query");
        req.setConversationId("conv-" + std::to_string(count_));
        req.setReplyWith ("reply-" + std::to_string(count_));

        std::cout << "[Alice → Bob] REQUEST #" << count_
                  << " : \"" << req.getContent() << "\"\n" << std::flush;

        if (!acl_send(target_, req))
            std::cerr << "[Alice] échec envoi #" << count_ << "\n";
    }

    bool done() override { return count_ >= MAX; }

    void onEnd() override {
        std::cout << "[Alice] envoi terminé (" << MAX << " messages)\n" << std::flush;
    }
};

// ── Behaviour d'Alice : reçoit les réponses INFORM de Bob ────────────────────

class AliceReceiveBehaviour : public Behaviour {
    int count_ = 0;
    static constexpr int MAX = 4;
public:
    explicit AliceReceiveBehaviour(Agent* ag) : Behaviour(ag) {}

    void action() override
    {
        auto opt = acl_receive("alice", 6000);
        if (!opt) {
            std::cout << "[Alice] timeout, plus de réponse attendue\n" << std::flush;
            count_ = MAX;  // abandon
            return;
        }
        const ACLMessage& msg = *opt;
        ++count_;
        std::cout << "[Alice ← Bob] "
                  << ACLMessage::performativeToString(msg.getPerformative())
                  << " : \"" << msg.getContent() << "\"\n" << std::flush;
    }

    bool done() override { return count_ >= MAX; }

    void onEnd() override {
        std::cout << "[Alice] réception terminée\n" << std::flush;
    }
};

// ── Behaviour de Bob : reçoit les REQUEST et répond INFORM ───────────────────

class BobReceiveBehaviour : public Behaviour {
    int count_ = 0;
    static constexpr int MAX = 4;
public:
    explicit BobReceiveBehaviour(Agent* ag) : Behaviour(ag) {}

    void action() override
    {
        auto opt = acl_receive("bob", 8000);
        if (!opt) {
            std::cout << "[Bob] timeout, arrêt\n" << std::flush;
            count_ = MAX;  // abandon
            return;
        }
        const ACLMessage& msg = *opt;
        ++count_;

        std::cout << "[Bob ← Alice] "
                  << ACLMessage::performativeToString(msg.getPerformative())
                  << " : \"" << msg.getContent() << "\"\n" << std::flush;

        // Construit la réponse INFORM avec l'heure courante
        std::time_t t = std::time(nullptr);
        char timebuf[32];
        std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", std::localtime(&t));

        ACLMessage reply = msg.createReply(ACLMessage::Performative::INFORM);
        reply.setSender (AgentIdentifier{"bob"});
        reply.setContent(std::string("Il est ") + timebuf);

        std::cout << "[Bob → Alice] INFORM : \"" << reply.getContent() << "\"\n"
                  << std::flush;

        acl_send(msg.getSender().name, reply);
    }

    bool done() override { return count_ >= MAX; }

    void onEnd() override {
        std::cout << "[Bob] traitement terminé\n" << std::flush;
    }
};

// ── Agents ────────────────────────────────────────────────────────────────────

class AliceAgent : public Agent {
public:
    void setup() override
    {
        std::cout << "[Alice] démarrage\n" << std::flush;
        addBehaviour(new AliceSendBehaviour   (this, "bob"));
        addBehaviour(new AliceReceiveBehaviour(this));
    }

    void takeDown() override
    {
        std::cout << "[Alice] arrêt propre\n" << std::flush;
        mq_unlink("/acl_alice");
    }
};

class BobAgent : public Agent {
public:
    void setup() override
    {
        std::cout << "[Bob] démarrage\n" << std::flush;
        addBehaviour(new BobReceiveBehaviour(this));
    }

    void takeDown() override
    {
        std::cout << "[Bob] arrêt propre\n" << std::flush;
        mq_unlink("/acl_bob");
    }
};

// ── Environnement minimal (pas de GUI) ───────────────────────────────────────

class DummyEnv : public Environnement {
public:
    void init_env()      override {}
    void link_attribut() override {}
    void event_loop()    override {}
};

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "======================================================\n";
    std::cout << " Échange ACL FIPA — Alice (REQUEST) ↔ Bob (INFORM)\n";
    std::cout << "======================================================\n\n";

    // Nettoyage des queues résiduelles
    mq_unlink("/acl_alice");
    mq_unlink("/acl_bob");

    AgentCore::initAgentSystem();

    BobAgent   bob;
    AliceAgent alice;

    bob.init();    // Bob démarre en premier pour que sa queue soit prête
    alice.init();  // Alice démarre ensuite

    // Attente de la fin des deux processus fils
    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();

    std::cout << "\n=== Démonstration terminée ===\n";
    return 0;
}
