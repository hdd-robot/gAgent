/*
 * Service.hpp — Entrée dans le Directory Facilitator
 *
 * Le DF (Directory Facilitator) maintient l'annuaire des services
 * offerts par les agents de la plateforme.
 */

#ifndef GAGENT_PLATFORM_SERVICE_HPP_
#define GAGENT_PLATFORM_SERVICE_HPP_

#include <string>
#include <sys/types.h>

namespace gagent {
namespace platform {

struct Service {
    std::string agentName;            // agent qui offre ce service
    std::string type;                 // type du service (ex: "planning")
    std::string name;                 // nom du service
    std::string language;             // langage du contenu (ex: "fipa-sl")
    std::string ontology;             // ontologie utilisée
    std::string interactionProtocol;  // protocole (ex: "fipa-request")
    std::string ownership;            // propriétaire
    pid_t       pid = 0;              // PID de l'agent (pour détection de mort)
    std::string slave_ip;             // IP de la plateforme esclave (vide = local)
};

} // namespace platform
} // namespace gagent

#endif
