/*
 * FipaAclDriver.hpp
 *
 * Orchestre le lexer Flex et le parser Bison pour FIPA ACL.
 */

#ifndef FIPA_ACL_DRIVER_HPP_
#define FIPA_ACL_DRIVER_HPP_

#include <string>
#include <optional>
#include "ACLMessage.hpp"

// Forward-déclaration du type location généré par Bison
namespace gagent {
    class FipaAclParser;
}
#include "fipa_acl_parser.hpp"

// Déclaration de la fonction helper définie dans fipa_acl.l (compilé en C++)
bool fipa_acl_parse_string(const std::string& input, gagent::FipaAclDriver& driver);

namespace gagent {

class FipaAclDriver {
public:
    // Résultat du parsing
    ACLMessage result;

    // Message d'erreur éventuel
    std::string parseError;

    // Position courante (mise à jour par le lexer)
    FipaAclParser::location_type loc;

    // Point d'entrée public
    std::optional<ACLMessage> parse(const std::string& input);
};

} // namespace gagent

#endif /* FIPA_ACL_DRIVER_HPP_ */
