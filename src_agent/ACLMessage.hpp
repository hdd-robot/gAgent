/*
 * ACLMessage.hpp
 *
 * FIPA ACL Message - refonte complète avec parser Flex/Bison
 */

#ifndef ACLMESSAGE_HPP_
#define ACLMESSAGE_HPP_

#include <string>
#include <vector>
#include <optional>

namespace gagent {

// Identifiant FIPA d'un agent dans un message ACL
struct AgentIdentifier {
    std::string name;
    std::vector<std::string> addresses;

    AgentIdentifier() = default;
    explicit AgentIdentifier(std::string n) : name(std::move(n)) {}
};

class ACLMessage {
public:
    enum class Performative {
        ACCEPT_PROPOSAL,
        AGREE,
        CANCEL,
        CFP,
        CONFIRM,
        DISCONFIRM,
        FAILURE,
        INFORM,
        INFORM_IF,
        INFORM_REF,
        NOT_UNDERSTOOD,
        PROPAGATE,
        PROPOSE,
        PROXY,
        QUERY_REF,
        REFUSE,
        REJECT_PROPOSAL,
        REQUEST,
        REQUEST_WHEN,
        REQUEST_WHENEVER,
        SUBSCRIBE,
        UNKNOWN
    };

    ACLMessage() = default;
    explicit ACLMessage(Performative p) : performative_(p) {}

    // Parse depuis une chaîne FIPA ACL
    static std::optional<ACLMessage> parse(const std::string& input);

    // Sérialise vers une chaîne FIPA ACL
    std::string toString() const;

    // Crée une réponse à ce message
    ACLMessage createReply(Performative p) const;

    // Setters
    void setPerformative(Performative p)       { performative_ = p; }
    void setSender(AgentIdentifier aid)        { sender_ = std::move(aid); }
    void addReceiver(AgentIdentifier aid)      { receivers_.push_back(std::move(aid)); }
    void setContent(std::string c)             { content_ = std::move(c); }
    void setLanguage(std::string l)            { language_ = std::move(l); }
    void setEncoding(std::string e)            { encoding_ = std::move(e); }
    void setOntology(std::string o)            { ontology_ = std::move(o); }
    void setProtocol(std::string p)            { protocol_ = std::move(p); }
    void setConversationId(std::string id)     { conversationId_ = std::move(id); }
    void setReplyWith(std::string rw)          { replyWith_ = std::move(rw); }
    void setInReplyTo(std::string irt)         { inReplyTo_ = std::move(irt); }

    // Getters
    Performative                         getPerformative()   const { return performative_; }
    const AgentIdentifier&               getSender()         const { return sender_; }
    const std::vector<AgentIdentifier>&  getReceivers()      const { return receivers_; }
    const std::string&                   getContent()        const { return content_; }
    const std::string&                   getLanguage()       const { return language_; }
    const std::string&                   getEncoding()       const { return encoding_; }
    const std::string&                   getOntology()       const { return ontology_; }
    const std::string&                   getProtocol()       const { return protocol_; }
    const std::string&                   getConversationId() const { return conversationId_; }
    const std::string&                   getReplyWith()      const { return replyWith_; }
    const std::string&                   getInReplyTo()      const { return inReplyTo_; }

    // Utilitaires
    static std::string    performativeToString(Performative p);
    static Performative   stringToPerformative(const std::string& s);

private:
    Performative               performative_{Performative::UNKNOWN};
    AgentIdentifier            sender_;
    std::vector<AgentIdentifier> receivers_;
    std::string                content_;
    std::string                language_;
    std::string                encoding_;
    std::string                ontology_;
    std::string                protocol_;
    std::string                conversationId_;
    std::string                replyWith_;
    std::string                inReplyTo_;
};

} // namespace gagent

#endif /* ACLMESSAGE_HPP_ */
