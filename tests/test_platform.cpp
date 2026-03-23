/*
 * test_platform.cpp — Tests d'intégration AMS + DF
 *
 * Requiert agentplatform en cours d'exécution.
 * Si la plateforme est absente, les tests sont ignorés (skip).
 *
 * Vérifie :
 *   AMS : register, double-register, lookup, setstate, deregister, list
 *   DF  : register service, search par type, search par type+ontologie,
 *         deregister, joker *
 */

#include <iostream>
#include <cassert>

#include <gagent/platform/AMSClient.hpp>
#include <gagent/platform/DFClient.hpp>

using namespace gagent::platform;

// ── Helpers ───────────────────────────────────────────────────────────────────

static int passed = 0;
static int failed = 0;
static int skipped = 0;

#define CHECK(cond, label) \
    do { \
        if (cond) { \
            std::cout << "  [OK]   " << label << "\n"; \
            ++passed; \
        } else { \
            std::cerr << "  [FAIL] " << label << "\n"; \
            ++failed; \
        } \
    } while(0)

// Vérifie si la plateforme est accessible
static bool platform_available()
{
    AMSClient c;
    auto r = c.list();   // retourne vide mais ne crash pas si dispo
    // Distinguer "dispo mais vide" de "non dispo" :
    // On tente un REGISTER bidon et regarde si on a une réponse
    bool ok = c.registerAgent("__probe__", -1, "/__probe__");
    if (ok) c.deregisterAgent("__probe__");
    // Si registerAgent retourne false ET list() retourne vide sans erreur,
    // ça peut être un double-register — mais __probe__ ne devrait pas exister.
    // On tente via DFClient aussi pour confirmer.
    DFClient df;
    df.deregisterAgent("__probe__");  // pas d'erreur même si absent
    return ok;  // false = plateforme non joignable
}

// ── Tests AMS ─────────────────────────────────────────────────────────────────

static void test_ams()
{
    std::cout << "\n── AMS\n";

    AMSClient ams;

    // register
    bool r1 = ams.registerAgent("test-alice", 1001, "/acl_test_alice");
    CHECK(r1, "registerAgent alice → OK");

    // double register → doit échouer
    bool r2 = ams.registerAgent("test-alice", 1001, "/acl_test_alice");
    CHECK(!r2, "double register → ERROR");

    // register bob
    ams.registerAgent("test-bob", 1002, "/acl_test_bob");

    // lookup existant
    auto info = ams.lookup("test-alice");
    CHECK(info.has_value(),              "lookup alice → trouvé");
    CHECK(info->name    == "test-alice", "lookup alice → nom correct");
    CHECK(info->pid     == 1001,         "lookup alice → pid correct");
    CHECK(info->address == "/acl_test_alice", "lookup alice → adresse correcte");
    CHECK(info->state   == "active",     "lookup alice → état active");

    // lookup inexistant
    auto miss = ams.lookup("agent-fantome");
    CHECK(!miss.has_value(), "lookup fantôme → nullopt");

    // setstate
    bool rs = ams.setState("test-alice", "suspended");
    CHECK(rs, "setState alice → suspended OK");
    auto info2 = ams.lookup("test-alice");
    CHECK(info2.has_value() && info2->state == "suspended",
          "lookup après setState → suspended");

    // list
    auto agents = ams.list();
    bool has_alice = false, has_bob = false;
    for (auto& a : agents) {
        if (a.name == "test-alice") has_alice = true;
        if (a.name == "test-bob")   has_bob   = true;
    }
    CHECK(has_alice, "list contient alice");
    CHECK(has_bob,   "list contient bob");

    // deregister
    bool rd = ams.deregisterAgent("test-alice");
    CHECK(rd, "deregisterAgent alice → OK");
    auto gone = ams.lookup("test-alice");
    CHECK(!gone.has_value(), "lookup après deregister → nullopt");

    // deregister inexistant
    bool rd2 = ams.deregisterAgent("agent-inexistant");
    CHECK(!rd2, "deregister inexistant → false");

    // nettoyage
    ams.deregisterAgent("test-bob");
}

// ── Tests DF ─────────────────────────────────────────────────────────────────

static void test_df()
{
    std::cout << "\n── DF\n";

    DFClient df;

    // register services
    bool r1 = df.registerService("test-alice", "planning",    "planner",    "fipa-sl", "logistics");
    bool r2 = df.registerService("test-bob",   "translation", "translator", "fipa-sl", "nlp");
    bool r3 = df.registerService("test-carol", "planning",    "planner2",   "fipa-sl", "routing");
    CHECK(r1 && r2 && r3, "registerService x3 → OK");

    // search par type
    auto planners = df.search("planning");
    CHECK(planners.size() >= 2, "search planning → ≥ 2 résultats");
    bool found_alice = false, found_carol = false;
    for (auto& s : planners) {
        if (s.agentName == "test-alice") found_alice = true;
        if (s.agentName == "test-carol") found_carol = true;
    }
    CHECK(found_alice, "search planning → alice présente");
    CHECK(found_carol, "search planning → carol présente");

    // search inexistant
    auto none = df.search("service-inexistant");
    CHECK(none.empty(), "search inexistant → vide");

    // search par type + ontologie
    auto logistics = df.search("planning", "logistics");
    CHECK(logistics.size() >= 1, "search planning+logistics → ≥ 1 résultat");
    bool only_alice = true;
    for (auto& s : logistics)
        if (s.agentName == "test-carol") only_alice = false;
    CHECK(only_alice, "search planning+logistics → pas carol (routing)");

    // joker *
    auto all = df.search("*");
    CHECK(all.size() >= 3, "search * → ≥ 3 résultats");

    // deregister
    df.deregisterAgent("test-alice");
    auto after = df.search("planning");
    bool alice_gone = true;
    for (auto& s : after)
        if (s.agentName == "test-alice") alice_gone = false;
    CHECK(alice_gone, "search planning après deregister alice → alice absente");

    // nettoyage
    df.deregisterAgent("test-bob");
    df.deregisterAgent("test-carol");
}

// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "=== Tests plateforme AMS + DF ===\n";
    std::cout << "(agentplatform doit être lancé)\n";

    if (!platform_available()) {
        std::cout << "\n[SKIP] agentplatform non disponible — tests ignorés\n";
        std::cout << "       Lancer './agentplatform' puis relancer ce test.\n";
        ++skipped;
        return 0;   // pas d'échec si plateforme absente
    }

    test_ams();
    test_df();

    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << passed << " OK, "
              << failed  << " FAIL, "
              << skipped << " SKIP\n";

    return failed > 0 ? 1 : 0;
}
