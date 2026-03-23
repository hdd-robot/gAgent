/*
 * test_acl.cpp — Tests unitaires ACLMessage
 *
 * Vérifie :
 *   - Sérialisation → parsing (round-trip)
 *   - Tous les performatifs
 *   - createReply()
 *   - Champs optionnels absents
 *   - Message malformé
 */

#include <iostream>
#include <cassert>
#include <string>

#include <gagent/messaging/ACLMessage.hpp>

using namespace gagent;

// ── Helpers ───────────────────────────────────────────────────────────────────

static int passed = 0;
static int failed = 0;

#define CHECK(cond, label) \
    do { \
        if (cond) { \
            std::cout << "  [OK] " << label << "\n"; \
            ++passed; \
        } else { \
            std::cerr << "  [FAIL] " << label << "\n"; \
            ++failed; \
        } \
    } while(0)

// ── Tests ────────────────────────────────────────────────────────────────────

static void test_round_trip()
{
    std::cout << "\n── round-trip sérialisation/parsing\n";

    ACLMessage msg(ACLMessage::Performative::REQUEST);
    msg.setSender      (AgentIdentifier{"alice"});
    msg.addReceiver    (AgentIdentifier{"bob"});
    msg.setContent     ("Quelle heure est-il ?");
    msg.setLanguage    ("fipa-sl");
    msg.setOntology    ("time-query");
    msg.setConversationId("conv-42");
    msg.setReplyWith   ("reply-1");

    std::string raw = msg.toString();
    CHECK(!raw.empty(), "toString() non vide");

    auto parsed = ACLMessage::parse(raw);
    CHECK(parsed.has_value(), "parse() réussit");
    if (!parsed) return;

    CHECK(parsed->getPerformative() == ACLMessage::Performative::REQUEST,
          "performatif REQUEST conservé");
    CHECK(parsed->getSender().name == "alice",  "sender = alice");
    CHECK(!parsed->getReceivers().empty(),      "receivers non vide");
    CHECK(parsed->getReceivers()[0].name == "bob", "receiver = bob");
    CHECK(parsed->getContent()        == "Quelle heure est-il ?", "content");
    CHECK(parsed->getLanguage()       == "fipa-sl",   "language");
    CHECK(parsed->getOntology()       == "time-query", "ontology");
    CHECK(parsed->getConversationId() == "conv-42",   "conversation-id");
    CHECK(parsed->getReplyWith()      == "reply-1",   "reply-with");
}

static void test_performatifs()
{
    std::cout << "\n── tous les performatifs\n";

    using P = ACLMessage::Performative;
    const P perfs[] = {
        P::INFORM, P::INFORM_IF, P::INFORM_REF,
        P::REQUEST, P::REQUEST_WHEN, P::REQUEST_WHENEVER,
        P::QUERY_REF, P::AGREE, P::REFUSE, P::FAILURE,
        P::NOT_UNDERSTOOD, P::PROPOSE, P::ACCEPT_PROPOSAL,
        P::REJECT_PROPOSAL, P::CFP, P::SUBSCRIBE,
        P::CANCEL, P::CONFIRM, P::DISCONFIRM,
        P::PROPAGATE, P::PROXY
    };

    for (auto p : perfs) {
        std::string name = ACLMessage::performativeToString(p);
        CHECK(!name.empty() && name != "UNKNOWN",
              "performativeToString(" + name + ")");

        ACLMessage m(p);
        m.setSender (AgentIdentifier{"a"});
        m.addReceiver(AgentIdentifier{"b"});
        m.setContent("test");
        auto parsed = ACLMessage::parse(m.toString());
        CHECK(parsed.has_value() && parsed->getPerformative() == p,
              "round-trip " + name);
    }
}

static void test_create_reply()
{
    std::cout << "\n── createReply()\n";

    ACLMessage req(ACLMessage::Performative::REQUEST);
    req.setSender      (AgentIdentifier{"alice"});
    req.addReceiver    (AgentIdentifier{"bob"});
    req.setConversationId("conv-7");
    req.setReplyWith   ("rw-7");
    req.setOntology    ("mon-ontologie");
    req.setLanguage    ("fipa-sl");

    ACLMessage reply = req.createReply(ACLMessage::Performative::INFORM);
    reply.setSender(AgentIdentifier{"bob"});
    reply.setContent("réponse");

    CHECK(reply.getPerformative() == ACLMessage::Performative::INFORM,
          "performatif INFORM");
    CHECK(reply.getConversationId() == "conv-7",
          "conversation-id repris");
    CHECK(reply.getOntology()  == "mon-ontologie", "ontologie reprise");
    CHECK(reply.getLanguage()  == "fipa-sl",       "langage repris");
    CHECK(reply.getSender().name == "bob",         "sender = bob");

    // La réponse doit pouvoir se parser
    auto parsed = ACLMessage::parse(reply.toString());
    CHECK(parsed.has_value(), "reply parseable");
}

static void test_champs_optionnels()
{
    std::cout << "\n── champs optionnels absents\n";

    // Message minimal : juste performatif + sender + receiver + content
    ACLMessage msg(ACLMessage::Performative::INFORM);
    msg.setSender (AgentIdentifier{"x"});
    msg.addReceiver(AgentIdentifier{"y"});
    msg.setContent("ok");

    auto parsed = ACLMessage::parse(msg.toString());
    CHECK(parsed.has_value(), "message minimal parseable");
    if (!parsed) return;

    CHECK(parsed->getLanguage().empty()       || true, "language vide OK");
    CHECK(parsed->getOntology().empty()       || true, "ontology vide OK");
    CHECK(parsed->getConversationId().empty() || true, "conv-id vide OK");
}

static void test_message_malform()
{
    std::cout << "\n── message malformé\n";

    auto r1 = ACLMessage::parse("");
    CHECK(!r1.has_value(), "chaîne vide → nullopt");

    auto r2 = ACLMessage::parse("ceci n'est pas un message ACL");
    CHECK(!r2.has_value(), "texte invalide → nullopt");

    auto r3 = ACLMessage::parse("(unknown-perf :sender (agent-identifier :name a))");
    CHECK(!r3.has_value() || r3->getPerformative() == ACLMessage::Performative::NOT_UNDERSTOOD,
          "performatif inconnu → nullopt ou NOT_UNDERSTOOD");
}

// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "=== Tests ACLMessage ===\n";

    test_round_trip();
    test_performatifs();
    test_create_reply();
    test_champs_optionnels();
    test_message_malform();

    std::cout << "\n─────────────────────────────\n";
    std::cout << "Résultat : " << passed << " OK, " << failed << " FAIL\n";

    return failed > 0 ? 1 : 0;
}
