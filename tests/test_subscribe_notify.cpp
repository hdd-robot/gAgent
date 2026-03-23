/*
 * test_subscribe_notify.cpp — Test du protocole FIPA Subscribe-Notify
 *
 * Scénario : capteur de température
 *
 *   SensorAgent (publisher) publie une température toutes les 200 ms
 *   MonitorAgent (subscriber) s'abonne et reçoit 3 notifications puis annule
 *
 * Flux observé :
 *   Monitor  → SUBSCRIBE  → Sensor
 *   Sensor   → AGREE      → Monitor
 *   Sensor   → INFORM 20° → Monitor   (tick 1)
 *   Sensor   → INFORM 21° → Monitor   (tick 2)
 *   Sensor   → INFORM 22° → Monitor   (tick 3)
 *   Monitor  → CANCEL     → Sensor
 *   Sensor   → INFORM fin → Monitor   (dernier message)
 */

#include <gagent/core/AgentCore.hpp>
#include <gagent/env/Environnement.hpp>
#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/protocols/SubscribeNotify.hpp>
#include <gagent/messaging/AclMQ.hpp>

#include <iostream>
#include <thread>
#include <chrono>
#include <sys/mman.h>

using namespace gagent;
using namespace gagent::protocols;
using namespace gagent::messaging;

// ── Compteurs partagés entre processus (mmap MAP_SHARED) ─────────────────────

struct SharedCounters {
    int  notify_count;
    bool refused;
};

static SharedCounters* g_shared = nullptr;

static void init_shared() {
    g_shared = static_cast<SharedCounters*>(
        mmap(nullptr, sizeof(SharedCounters),
             PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    g_shared->notify_count = 0;
    g_shared->refused      = false;
}

// ── Environnement minimal ─────────────────────────────────────────────────────

class DummyEnv : public Environnement {
public:
    void init_env()      override {}
    void link_attribut() override {}
    void event_loop()    override {}
};

class TempSubscriber : public SubscribeInitiator {
    int max_notify_;
public:
    TempSubscriber(Agent* ag, const std::string& my_name,
                   const std::string& publisher, int max_notify)
        : SubscribeInitiator(ag, my_name, publisher, "temperature", "sensors")
        , max_notify_(max_notify)
    {}

    void handleNotify(const ACLMessage& msg) override {
        int n = ++g_shared->notify_count;
        std::cout << "[Monitor] notification #" << n
                  << " : " << msg.getContent() << "\n" << std::flush;
    }

    void handleRefuse(const ACLMessage& msg) override {
        g_shared->refused = true;
        std::cout << "[Monitor] abonnement refusé : " << msg.getContent() << "\n";
    }

    bool shouldCancel() override {
        return g_shared->notify_count >= max_notify_;
    }

    void onEnd() override {
        std::cout << "[Monitor] protocole terminé ("
                  << g_shared->notify_count << " notifications)\n";
        this_agent->doDelete();
    }
};

// ── Publisher ─────────────────────────────────────────────────────────────────

class TempPublisher : public SubscribeParticipant {
    int temp_     = 20;
    int tick_     = 0;
    int max_tick_ = 20;   // sécurité — arrêt au bout de 4 s
public:
    TempPublisher(Agent* ag, const std::string& my_name)
        : SubscribeParticipant(ag, my_name, 200)
    {}

    bool handleSubscribe(const ACLMessage& msg) override {
        std::cout << "[Sensor] abonnement accepté de "
                  << msg.getSender().name << "\n" << std::flush;
        return true;
    }

    std::string handleCancel(const std::string& sub) override {
        std::cout << "[Sensor] CANCEL de " << sub << "\n" << std::flush;
        return "fin-abonnement";
    }

    // Surcharge action() pour publier périodiquement
    void action() override {
        SubscribeParticipant::action();  // traiter SUBSCRIBE / CANCEL entrants

        if (subscriberCount() > 0 && (tick_ % 5 == 0) && tick_ > 0) {
            std::string val = std::to_string(temp_++) + "°C";
            std::cout << "[Sensor] publie " << val << "\n" << std::flush;
            notify(val, "sensors");
        }
        ++tick_;

        if (tick_ >= max_tick_) {
            this_agent->doDelete();
        }
    }

    bool done() override { return false; }

    void onEnd() override {
        std::cout << "[Sensor] arrêt\n";
        this_agent->doDelete();
    }
};

// ── Agents ────────────────────────────────────────────────────────────────────

class SensorAgent : public Agent {
public:
    void setup() override {
        std::cout << "[Sensor] démarrage\n";
        addBehaviour(new TempPublisher(this, "sensor"));
    }
    void takeDown() override { acl_unlink("sensor"); }
};

class MonitorAgent : public Agent {
public:
    void setup() override {
        std::cout << "[Monitor] démarrage\n";
        addBehaviour(new TempSubscriber(this, "monitor", "sensor", 3));
    }
    void takeDown() override { acl_unlink("monitor"); }
};

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "======================================================\n"
              << " FIPA Subscribe-Notify — capteur de température\n"
              << "======================================================\n\n";

    init_shared();

    acl_unlink("sensor");
    acl_unlink("monitor");

    AgentCore::initAgentSystem();

    DummyEnv env;
    AgentCore::initEnvironnementSystem(env);

    SensorAgent  sensor;
    MonitorAgent monitor;

    sensor.init();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    monitor.init();

    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();

    std::cout << "\n=== Résultat ===\n";
    std::cout << "  notifications reçues : " << g_shared->notify_count << "\n";
    std::cout << "  abonnement refusé    : " << (g_shared->refused ? "oui" : "non") << "\n";

    bool ok = (g_shared->notify_count >= 3 && !g_shared->refused);
    std::cout << (ok ? "\n[OK] Subscribe-Notify fonctionne\n"
                     : "\n[FAIL] résultat inattendu\n");
    return ok ? 0 : 1;
}
