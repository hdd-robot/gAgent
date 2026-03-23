/*
 * ACLMessage.cpp
 */

#include <gagent/messaging/ACLMessage.hpp>
#include "FipaAclDriver.hpp"
#include <sstream>

namespace gagent {

/* ── parse ───────────────────────────────────────────────── */

std::optional<ACLMessage> ACLMessage::parse(const std::string& input)
{
    FipaAclDriver driver;
    return driver.parse(input);
}

/* ── toString ────────────────────────────────────────────── */

static std::string escapeString(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 2);
    out += '"';
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    out += '"';
    return out;
}

static std::string agentIdToString(const AgentIdentifier& aid)
{
    std::ostringstream os;
    os << "(agent-identifier :name " << aid.name;
    if (!aid.addresses.empty()) {
        os << " :addresses (sequence";
        for (const auto& a : aid.addresses)
            os << " " << a;
        os << ")";
    }
    os << ")";
    return os.str();
}

std::string ACLMessage::toString() const
{
    std::ostringstream os;
    os << "(" << performativeToString(performative_) << "\n";

    if (!sender_.name.empty())
        os << " :sender "   << agentIdToString(sender_) << "\n";

    for (const auto& r : receivers_)
        os << " :receiver " << agentIdToString(r) << "\n";

    if (!content_.empty())
        os << " :content "       << escapeString(content_)  << "\n";
    if (!language_.empty())
        os << " :language "      << language_               << "\n";
    if (!encoding_.empty())
        os << " :encoding "      << encoding_               << "\n";
    if (!ontology_.empty())
        os << " :ontology "      << ontology_               << "\n";
    if (!protocol_.empty())
        os << " :protocol "      << protocol_               << "\n";
    if (!conversationId_.empty())
        os << " :conversation-id " << conversationId_       << "\n";
    if (!replyWith_.empty())
        os << " :reply-with "    << replyWith_              << "\n";
    if (!inReplyTo_.empty())
        os << " :in-reply-to "   << inReplyTo_              << "\n";

    os << ")";
    return os.str();
}

/* ── createReply ─────────────────────────────────────────── */

ACLMessage ACLMessage::createReply(Performative p) const
{
    ACLMessage reply(p);
    // Inverse sender/receiver
    reply.setSender(receivers_.empty() ? AgentIdentifier{} : receivers_.front());
    reply.addReceiver(sender_);
    reply.setLanguage(language_);
    reply.setOntology(ontology_);
    reply.setProtocol(protocol_);
    reply.setConversationId(conversationId_);
    if (!replyWith_.empty())
        reply.setInReplyTo(replyWith_);
    return reply;
}

/* ── performativeToString ────────────────────────────────── */

std::string ACLMessage::performativeToString(Performative p)
{
    switch (p) {
        case Performative::ACCEPT_PROPOSAL:  return "accept-proposal";
        case Performative::AGREE:            return "agree";
        case Performative::CANCEL:           return "cancel";
        case Performative::CFP:              return "cfp";
        case Performative::CONFIRM:          return "confirm";
        case Performative::DISCONFIRM:       return "disconfirm";
        case Performative::FAILURE:          return "failure";
        case Performative::INFORM:           return "inform";
        case Performative::INFORM_IF:        return "inform-if";
        case Performative::INFORM_REF:       return "inform-ref";
        case Performative::NOT_UNDERSTOOD:   return "not-understood";
        case Performative::PROPAGATE:        return "propagate";
        case Performative::PROPOSE:          return "propose";
        case Performative::PROXY:            return "proxy";
        case Performative::QUERY_REF:        return "query-ref";
        case Performative::REFUSE:           return "refuse";
        case Performative::REJECT_PROPOSAL:  return "reject-proposal";
        case Performative::REQUEST:          return "request";
        case Performative::REQUEST_WHEN:     return "request-when";
        case Performative::REQUEST_WHENEVER: return "request-whenever";
        case Performative::SUBSCRIBE:        return "subscribe";
        default:                             return "unknown";
    }
}

/* ── stringToPerformative ────────────────────────────────── */

ACLMessage::Performative ACLMessage::stringToPerformative(const std::string& s)
{
    if (s == "accept-proposal")  return Performative::ACCEPT_PROPOSAL;
    if (s == "agree")            return Performative::AGREE;
    if (s == "cancel")           return Performative::CANCEL;
    if (s == "cfp")              return Performative::CFP;
    if (s == "confirm")          return Performative::CONFIRM;
    if (s == "disconfirm")       return Performative::DISCONFIRM;
    if (s == "failure")          return Performative::FAILURE;
    if (s == "inform")           return Performative::INFORM;
    if (s == "inform-if")        return Performative::INFORM_IF;
    if (s == "inform-ref")       return Performative::INFORM_REF;
    if (s == "not-understood")   return Performative::NOT_UNDERSTOOD;
    if (s == "propagate")        return Performative::PROPAGATE;
    if (s == "propose")          return Performative::PROPOSE;
    if (s == "proxy")            return Performative::PROXY;
    if (s == "query-ref")        return Performative::QUERY_REF;
    if (s == "refuse")           return Performative::REFUSE;
    if (s == "reject-proposal")  return Performative::REJECT_PROPOSAL;
    if (s == "request")          return Performative::REQUEST;
    if (s == "request-when")     return Performative::REQUEST_WHEN;
    if (s == "request-whenever") return Performative::REQUEST_WHENEVER;
    if (s == "subscribe")        return Performative::SUBSCRIBE;
    return Performative::UNKNOWN;
}

} // namespace gagent
