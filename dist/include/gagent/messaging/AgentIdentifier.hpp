/*
 * AgentIdentifier.hpp
 *
 * FIPA Agent Identifier - extracted from ACLMessage
 */

#ifndef GAGENT_AGENTIDENTIFIER_HPP_
#define GAGENT_AGENTIDENTIFIER_HPP_

#include <string>
#include <vector>

namespace gagent {

// Identifiant FIPA d'un agent dans un message ACL
struct AgentIdentifier {
    std::string name;
    std::vector<std::string> addresses;

    AgentIdentifier() = default;
    explicit AgentIdentifier(std::string n) : name(std::move(n)) {}
};

} // namespace gagent

#endif /* GAGENT_AGENTIDENTIFIER_HPP_ */
