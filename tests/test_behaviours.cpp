/*
 * test_behaviours.cpp — Tests de régression du cycle de vie des behaviours
 *
 * Scénarios :
 *   1. OneShotBehaviour        : action() exécutée exactement une fois
 *   2. removeBehaviour()       : le behaviour retiré n'est pas exécuté
 *   3. getAttribut()           : lecture après setAttribut()
 *   4. ParallelBehaviour       : enfants OneShot exécutés puis terminés
 *   5. SequentialBehaviour     : enfants OneShot exécutés dans l'ordre
 *
 * Les scénarios 1 et 2 passent par l'ordonnanceur de l'agent (fork) :
 * les compteurs sont partagés via mmap(MAP_SHARED|MAP_ANONYMOUS).
 */

#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>

#include <iostream>
#include <string>
#include <sys/mman.h>
#include <cstring>

using namespace gagent;

// ── Données partagées avec les processus agents ───────────────────────────────

struct Shared {
    int oneshot_runs;
    int removed_runs;
    int kept_runs;
};

static Shared* g_shared = nullptr;

static void init_shared() {
    g_shared = static_cast<Shared*>(
        mmap(nullptr, sizeof(Shared),
             PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    memset(g_shared, 0, sizeof(Shared));
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static int g_ok = 0, g_fail = 0;

// Vide le tampon stdio avant fork() : sinon le processus agent hérite du
// tampon non vidé du parent et l'affiche une seconde fois.
static void flush_before_fork() { std::cout.flush(); }

static void check(bool cond, const std::string& label) {
    std::cout << "  [" << (cond ? "OK  " : "FAIL") << "] " << label << "\n";
    cond ? g_ok++ : g_fail++;
}

// ── Scénario 1 : OneShotBehaviour ─────────────────────────────────────────────

class CountingOneShot : public OneShotBehaviour {
public:
    explicit CountingOneShot(Agent* ag) : OneShotBehaviour(ag) {}
    void action() override { g_shared->oneshot_runs++; }
};

class OneShotAgent : public Agent {
public:
    void setup() override { addBehaviour(new CountingOneShot(this)); }
};

// ── Scénario 2 : removeBehaviour ──────────────────────────────────────────────

class RemovedBeh : public OneShotBehaviour {
public:
    explicit RemovedBeh(Agent* ag) : OneShotBehaviour(ag) {}
    void action() override { g_shared->removed_runs++; }
};

class KeptBeh : public OneShotBehaviour {
public:
    explicit KeptBeh(Agent* ag) : OneShotBehaviour(ag) {}
    void action() override { g_shared->kept_runs++; }
};

class RemoveAgent : public Agent {
public:
    void setup() override {
        Behaviour* removed = new RemovedBeh(this);
        addBehaviour(removed);
        addBehaviour(new KeptBeh(this));
        removeBehaviour(removed);
        delete removed;
    }
};

// ── Scénario 3 : attributs ────────────────────────────────────────────────────

class AttrAgent : public Agent {
public:
    void setup() override {}
};

// ── Scénarios 4 et 5 : composites pilotés à la main (sans fork) ───────────────

class LocalOneShot : public OneShotBehaviour {
public:
    LocalOneShot(Agent* ag, int* counter) : OneShotBehaviour(ag), counter_(counter) {}
    void action() override { (*counter_)++; }
private:
    int* counter_;
};

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "======================================================\n"
              << " Behaviours — cycle de vie\n"
              << "======================================================\n\n";

    init_shared();
    AgentCore::initAgentSystem();

    // ── Scénario 1 ────────────────────────────────────────────────────────────
    std::cout << "--- Scénario 1 : OneShotBehaviour ---\n";
    {
        OneShotAgent ag;
        flush_before_fork();
        ag.init();
        AgentCore::syncAgentSystem();
    }
    check(g_shared->oneshot_runs == 1,
          "action() exécutée exactement une fois (obtenu : "
          + std::to_string(g_shared->oneshot_runs) + ")");

    // ── Scénario 2 ────────────────────────────────────────────────────────────
    std::cout << "\n--- Scénario 2 : removeBehaviour() ---\n";
    {
        RemoveAgent ag;
        flush_before_fork();
        ag.init();
        AgentCore::syncAgentSystem();
    }
    check(g_shared->removed_runs == 0, "le behaviour retiré n'a pas été exécuté");
    check(g_shared->kept_runs    == 1, "le behaviour conservé a été exécuté");

    // ── Scénario 3 ────────────────────────────────────────────────────────────
    std::cout << "\n--- Scénario 3 : getAttribut() ---\n";
    {
        AttrAgent ag;
        ag.addAttribut("couleur");
        ag.setAttribut("couleur", "rouge");
        check(ag.getAttribut("couleur") == "rouge", "relecture de l'attribut posé");
        check(ag.getAttribut("inconnu").empty(),    "attribut absent → chaîne vide");

        ag.removeAttribut("couleur");
        check(ag.getAttribut("couleur").empty(),    "attribut retiré → chaîne vide");
    }

    // ── Scénario 4 ────────────────────────────────────────────────────────────
    std::cout << "\n--- Scénario 4 : ParallelBehaviour + OneShot ---\n";
    {
        int a = 0, b = 0;
        ParallelBehaviour par(nullptr, ParallelBehaviour::WhenDone::ALL);
        par.addSubBehaviour(new LocalOneShot(nullptr, &a));
        par.addSubBehaviour(new LocalOneShot(nullptr, &b));

        par.onStart();
        check(!par.done(), "non terminé avant la première action()");
        par.action();
        check(a == 1 && b == 1, "les deux enfants ont été exécutés une fois");
        check(par.done(),       "terminé après exécution de tous les enfants");
        par.action();
        check(a == 1 && b == 1, "aucune ré-exécution après terminaison");
    }

    // ── Scénario 5 ────────────────────────────────────────────────────────────
    std::cout << "\n--- Scénario 5 : SequentialBehaviour + OneShot ---\n";
    {
        int a = 0, b = 0;
        SequentialBehaviour seq(nullptr);
        seq.addSubBehaviour(new LocalOneShot(nullptr, &a));
        seq.addSubBehaviour(new LocalOneShot(nullptr, &b));

        seq.onStart();
        seq.action();
        check(a == 1 && b == 0, "premier enfant exécuté seul");
        seq.action();
        check(a == 1 && b == 1, "second enfant exécuté ensuite");
        check(seq.done(),       "terminé après le dernier enfant");
    }

    // ── Bilan ─────────────────────────────────────────────────────────────────
    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << g_ok << " OK, " << g_fail << " FAIL\n\n";

    if (g_fail == 0) { std::cout << "[OK] Behaviours conformes\n"; return 0; }
    std::cout << "[FAIL] Erreurs détectées\n";
    return 1;
}
