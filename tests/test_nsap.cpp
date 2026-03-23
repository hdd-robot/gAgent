/*
 * test_nsap.cpp
 *
 * Valide la pile de snapshots (NSAP) de l'Environnement.
 *
 * Scénario :
 *   t0 : agent1 à (10, 20)
 *   push → snap #0
 *   t1 : agent1 bouge à (50, 80), agent2 apparaît
 *   push → snap #1
 *   pull → retour à t1 (agent1 en 50,80 ; agent2 présent)
 *   pull → retour à t0 (agent1 en 10,20 ; agent2 absent)
 *   pull → pile vide → retourne -1
 *   clear_nsap → pile vidée
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <string>
#include <mqueue.h>

#include <gagent/env/Environnement.hpp>

using namespace gagent;

// Envoie un état d'agent dans la queue /envqueuemsg (simule attributUpdated())
static void mq_inject(const std::string& attrs_str)
{
    const std::string mq_name = "/envqueuemsg";
    const int taille  = 1000;
    const int max_msg = 10;

    struct mq_attr attr{};
    attr.mq_maxmsg  = max_msg;
    attr.mq_msgsize = taille;

    mqd_t mq = mq_open(mq_name.c_str(), O_WRONLY | O_CREAT, 0666, &attr);
    if (mq == (mqd_t)-1) { perror("mq_inject"); return; }
    mq_send(mq, attrs_str.c_str(), attrs_str.size(), 0);
    mq_close(mq);
}

static void wait() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }

// Retrouve un VisualAgent par id dans la liste
static VisualAgent* find(Environnement& env, const std::string& id) {
    for (auto* va : env.list_visual_agents)
        if (va->id == id) return va;
    return nullptr;
}

int main()
{
    std::cout << "=== Test NSAP (pile de snapshots) ===\n\n";

    mq_unlink("/envqueuemsg");

    Environnement env;
    env.link_id   ("id");
    env.link_pos_x("x");
    env.link_pos_y("y");
    env.link_name ("name");

    // Lance la lecture MQ en arrière-plan (comme start() sans GUI)
    std::thread reader(&Environnement::readDataFromQueueMsg, &env);
    reader.detach();
    wait();

    // ── t0 : agent1 à (10, 20) ───────────────────────────────────────────
    std::cout << "── t0 : agent1 à (10, 20)\n";
    mq_inject("id:agent1;name:agent1;x:10;y:20;");
    wait();

    env.make_agent();
    assert(env.list_visual_agents.size() == 1);
    assert(find(env, "agent1")->pos_x == 10.0f);
    std::cout << "  agent1.x = " << find(env, "agent1")->pos_x << " ✓\n";

    // push #0
    int r = env.push_nsap();
    assert(r == 1);
    assert(env.get_nsaps()->size() == 1);
    std::cout << "  push #0 → pile = " << r << " ✓\n\n";

    // ── t1 : agent1 → (50, 80), agent2 apparaît ─────────────────────────
    std::cout << "── t1 : agent1→(50,80), agent2 apparaît\n";
    mq_inject("id:agent1;name:agent1;x:50;y:80;");
    mq_inject("id:agent2;name:agent2;x:100;y:100;");
    wait();

    env.make_agent();
    assert(env.list_visual_agents.size() == 2);
    assert(find(env, "agent1")->pos_x == 50.0f);
    assert(find(env, "agent2") != nullptr);
    std::cout << "  2 agents ✓, agent1.x = 50 ✓\n";

    // push #1
    r = env.push_nsap();
    assert(r == 2);
    assert(env.get_nsaps()->size() == 2);
    std::cout << "  push #1 → pile = " << r << " ✓\n\n";

    // ── pull → retour t1 ─────────────────────────────────────────────────
    std::cout << "── pull → retour t1\n";
    r = env.pull_nsap();
    assert(r == 1);
    env.make_agent();
    assert(env.list_visual_agents.size() == 2);
    assert(find(env, "agent1")->pos_x == 50.0f);
    assert(find(env, "agent2") != nullptr);
    std::cout << "  agent1.x = 50 ✓, agent2 présent ✓\n\n";

    // ── pull → retour t0 ─────────────────────────────────────────────────
    std::cout << "── pull → retour t0\n";
    r = env.pull_nsap();
    assert(r == 0);
    env.make_agent();
    assert(env.list_visual_agents.size() == 1);
    assert(find(env, "agent1")->pos_x == 10.0f);
    assert(find(env, "agent2") == nullptr);
    std::cout << "  agent1.x = 10 ✓, agent2 absent ✓\n\n";

    // ── pull sur pile vide → -1 ───────────────────────────────────────────
    std::cout << "── pull sur pile vide\n";
    r = env.pull_nsap();
    assert(r == -1);
    std::cout << "  retourne -1 ✓\n\n";

    // ── clear_nsap ────────────────────────────────────────────────────────
    std::cout << "── clear_nsap\n";
    env.push_nsap();
    env.push_nsap();
    assert(env.get_nsaps()->size() == 2);
    env.clear_nsap();
    assert(env.get_nsaps()->empty());
    std::cout << "  pile vidée ✓\n\n";

    std::cout << "=== Tous les tests NSAP passent ✓ ===\n";

    mq_unlink("/envqueuemsg");
    return 0;
}
