/*
 * test_subscribe_notify.cpp — Test du protocole FIPA Subscribe-Notify
 *
 * Trois scénarios :
 *
 *   1. Succès    : monitor s'abonne à sensor, reçoit 3 notifs, envoie CANCEL
 *                  sensor confirme avec un dernier INFORM post-CANCEL
 *
 *   2. Refus     : monitor2 s'abonne à sensor2 qui refuse (REFUSE)
 *                  handleRefuse() appelé, protocole terminé
 *
 *   3. Multi-sub : monitor3a et monitor3b s'abonnent à sensor3
 *                  sensor3 attend 2 abonnés avant de publier
 *                  les 2 subscribers reçoivent chacun les notifications
 */

#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
#include <gagent/protocols/SubscribeNotify.hpp>
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
    // Scénario 1
    int  notify_count;
    bool refused;
    bool cancel_sent;
    bool last_inform;
    // Scénario 2
    bool refused2;
    int  notify_count2;   // doit rester 0
    // Scénario 3
    int  notify_count_a;  // notifications reçues par monitor3a
    int  notify_count_b;  // notifications reçues par monitor3b
};

static Shared* g_shared = nullptr;

static void init_shared() {
    g_shared = static_cast<Shared*>(
        mmap(nullptr, sizeof(Shared),
             PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    memset(g_shared, 0, sizeof(Shared));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Scénario 1 : succès + CANCEL
// ═══════════════════════════════════════════════════════════════════════════════

class TempSubscriber : public SubscribeInitiator {
    int max_notify_;
public:
    TempSubscriber(Agent* ag, const std::string& my_name,
                   const std::string& publisher, int max_notify)
        : SubscribeInitiator(ag, my_name, publisher, "temperature", "sensors")
        , max_notify_(max_notify) {}

    void handleNotify(const ACLMessage& msg) override {
        int n = ++g_shared->notify_count;
        std::cout << "[Monitor] #" << n << " : " << msg.getContent() << "\n" << std::flush;
        if (g_shared->cancel_sent)
            g_shared->last_inform = true;
    }
    void handleRefuse(const ACLMessage& /*m*/) override { g_shared->refused = true; }
    bool shouldCancel() override {
        if (g_shared->notify_count >= max_notify_) { g_shared->cancel_sent = true; return true; }
        return false;
    }
    void onEnd() override { this_agent->doDelete(); }
};

class TempPublisher : public SubscribeParticipant {
    int temp_ = 20, tick_ = 0, max_tick_ = 25;
public:
    TempPublisher(Agent* ag, const std::string& name)
        : SubscribeParticipant(ag, name, 200) {}

    bool handleSubscribe(const ACLMessage& msg) override {
        std::cout << "[Sensor] abonnement de " << msg.getSender().name << "\n" << std::flush;
        return true;
    }
    std::string handleCancel(const std::string& sub) override {
        std::cout << "[Sensor] CANCEL de " << sub << "\n" << std::flush;
        return "fin-abonnement";
    }
    void action() override {
        SubscribeParticipant::action();
        if (subscriberCount() > 0 && (tick_ % 5 == 0) && tick_ > 0) {
            std::string val = std::to_string(temp_++) + "°C";
            notify(val, "sensors");
        }
        if (++tick_ >= max_tick_) this_agent->doDelete();
    }
    bool done()  override { return false; }
    void onEnd() override { this_agent->doDelete(); }
};

class SensorAgent : public Agent {
public:
    void setup()    override { addBehaviour(new TempPublisher(this, "sensor")); }
    void takeDown() override { acl_unlink("sensor"); }
};

class MonitorAgent : public Agent {
public:
    void setup()    override { addBehaviour(new TempSubscriber(this, "monitor", "sensor", 3)); }
    void takeDown() override { acl_unlink("monitor"); }
};

// ═══════════════════════════════════════════════════════════════════════════════
// Scénario 2 : abonnement refusé
// ═══════════════════════════════════════════════════════════════════════════════

class RefusedSubscriber : public SubscribeInitiator {
public:
    RefusedSubscriber(Agent* ag)
        : SubscribeInitiator(ag, "monitor2", "sensor2", "secret", "", 3000) {}

    void handleNotify(const ACLMessage& /*m*/) override { g_shared->notify_count2++; }
    void handleRefuse(const ACLMessage& /*m*/) override { g_shared->refused2 = true; }
    void onEnd() override { this_agent->doDelete(); }
};

class RefusingPublisher : public SubscribeParticipant {
    int tick_ = 0;
public:
    RefusingPublisher(Agent* ag)
        : SubscribeParticipant(ag, "sensor2", 200) {}

    bool handleSubscribe(const ACLMessage& /*msg*/) override { return false; } // refuse tout
    void action() override {
        SubscribeParticipant::action();
        if (++tick_ >= 20) this_agent->doDelete();
    }
    bool done()  override { return false; }
    void onEnd() override { this_agent->doDelete(); }
};

class Sensor2Agent : public Agent {
public:
    void setup()    override { addBehaviour(new RefusingPublisher(this)); }
    void takeDown() override { acl_unlink("sensor2"); }
};

class Monitor2Agent : public Agent {
public:
    void setup()    override { addBehaviour(new RefusedSubscriber(this)); }
    void takeDown() override { acl_unlink("monitor2"); }
};

// ═══════════════════════════════════════════════════════════════════════════════
// Scénario 3 : multi-abonnés
// ═══════════════════════════════════════════════════════════════════════════════

class MultiSubscriberA : public SubscribeInitiator {
    int max_;
public:
    MultiSubscriberA(Agent* ag, int max)
        : SubscribeInitiator(ag, "monitor3a", "sensor3", "data", "", 5000)
        , max_(max) {}

    void handleNotify(const ACLMessage& msg) override {
        int n = ++g_shared->notify_count_a;
        std::cout << "[Monitor3A] #" << n << " : " << msg.getContent() << "\n" << std::flush;
    }
    bool shouldCancel() override { return g_shared->notify_count_a >= max_; }
    void onEnd() override { this_agent->doDelete(); }
};

class MultiSubscriberB : public SubscribeInitiator {
    int max_;
public:
    MultiSubscriberB(Agent* ag, int max)
        : SubscribeInitiator(ag, "monitor3b", "sensor3", "data", "", 5000)
        , max_(max) {}

    void handleNotify(const ACLMessage& msg) override {
        int n = ++g_shared->notify_count_b;
        std::cout << "[Monitor3B] #" << n << " : " << msg.getContent() << "\n" << std::flush;
    }
    bool shouldCancel() override { return g_shared->notify_count_b >= max_; }
    void onEnd() override { this_agent->doDelete(); }
};

// Publisher qui attend d'avoir 2 abonnés avant de publier
class MultiPublisher : public SubscribeParticipant {
    int val_ = 0, tick_ = 0, max_tick_ = 40;
public:
    MultiPublisher(Agent* ag)
        : SubscribeParticipant(ag, "sensor3", 200) {}

    bool handleSubscribe(const ACLMessage& msg) override {
        std::cout << "[Sensor3] abonnement de " << msg.getSender().name << "\n" << std::flush;
        return true;
    }
    std::string handleCancel(const std::string& sub) override {
        std::cout << "[Sensor3] CANCEL de " << sub << "\n" << std::flush;
        return "";
    }
    void action() override {
        SubscribeParticipant::action();
        // Publier seulement quand 2 abonnés sont présents
        if (subscriberCount() >= 2 && (tick_ % 5 == 0) && tick_ > 0) {
            notify("v" + std::to_string(val_++), "sensors");
        }
        if (++tick_ >= max_tick_) this_agent->doDelete();
    }
    bool done()  override { return false; }
    void onEnd() override { this_agent->doDelete(); }
};

class Sensor3Agent : public Agent {
public:
    void setup()    override { addBehaviour(new MultiPublisher(this)); }
    void takeDown() override { acl_unlink("sensor3"); }
};

class Monitor3aAgent : public Agent {
public:
    void setup()    override { addBehaviour(new MultiSubscriberA(this, 3)); }
    void takeDown() override { acl_unlink("monitor3a"); }
};

class Monitor3bAgent : public Agent {
public:
    void setup()    override { addBehaviour(new MultiSubscriberB(this, 3)); }
    void takeDown() override { acl_unlink("monitor3b"); }
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
              << " FIPA Subscribe-Notify — trois scénarios\n"
              << "======================================================\n\n";

    init_shared();

    for (auto n : {"sensor","monitor","sensor2","monitor2",
                   "sensor3","monitor3a","monitor3b"})
        acl_unlink(n);

    AgentCore::initAgentSystem();

    int ok = 0, fail = 0;

    // ── Scénario 1 : succès + CANCEL ─────────────────────────────────────────
    std::cout << "--- Scénario 1 : succès + CANCEL ---\n";
    {
        SensorAgent  sensor;
        MonitorAgent monitor;
        sensor.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        monitor.init();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
    }
    run_check(g_shared->notify_count >= 3,  "≥ 3 notifications reçues",           ok, fail);
    run_check(!g_shared->refused,           "abonnement non refusé",               ok, fail);
    run_check(g_shared->cancel_sent,        "CANCEL envoyé après 3 notifications", ok, fail);
    run_check(g_shared->last_inform,        "INFORM post-CANCEL reçu",             ok, fail);

    // ── Scénario 2 : abonnement refusé ───────────────────────────────────────
    std::cout << "\n--- Scénario 2 : abonnement refusé ---\n";
    {
        Sensor2Agent  sensor2;
        Monitor2Agent monitor2;
        sensor2.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        monitor2.init();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
    }
    run_check(g_shared->refused2,           "monitor2 a reçu REFUSE",             ok, fail);
    run_check(g_shared->notify_count2 == 0, "aucune notification après REFUSE",   ok, fail);

    // ── Scénario 3 : multi-abonnés ────────────────────────────────────────────
    std::cout << "\n--- Scénario 3 : fan-out (2 abonnés) ---\n";
    {
        Sensor3Agent   sensor3;
        Monitor3aAgent monitor3a;
        Monitor3bAgent monitor3b;
        sensor3.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        monitor3a.init();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        monitor3b.init();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
        AgentCore::syncAgentSystem();
    }
    std::cout << "  notify_count_a=" << g_shared->notify_count_a
              << " notify_count_b=" << g_shared->notify_count_b << "\n";
    run_check(g_shared->notify_count_a >= 3, "monitor3a a reçu ≥ 3 notifications", ok, fail);
    run_check(g_shared->notify_count_b >= 3, "monitor3b a reçu ≥ 3 notifications", ok, fail);
    run_check(g_shared->notify_count_a == g_shared->notify_count_b ||
              abs(g_shared->notify_count_a - g_shared->notify_count_b) <= 1,
              "fan-out symétrique (±1 notif entre les deux)",       ok, fail);

    // ── Bilan ─────────────────────────────────────────────────────────────────
    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << ok << " OK, " << fail << " FAIL\n\n";

    if (fail == 0) { std::cout << "[OK] Subscribe-Notify fonctionne\n"; return 0; }
    std::cout << "[FAIL] Erreurs détectées\n";
    return 1;
}
