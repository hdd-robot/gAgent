/*
 * DFClient.hpp — Client du Directory Facilitator
 *
 * Permet à un agent d'annoncer ses services et de chercher
 * des agents offrant un service donné.
 *
 * Usage type dans Agent::setup() :
 *   DFClient df;
 *   df.registerService("alice", "planning", "my-planner",
 *                      "fipa-sl", "logistics");
 */

#ifndef GAGENT_PLATFORM_DFCLIENT_HPP_
#define GAGENT_PLATFORM_DFCLIENT_HPP_

#include <string>
#include <vector>

namespace gagent {
namespace platform {

struct ServiceInfo {
    std::string agentName;
    std::string type;
    std::string serviceName;
    std::string language;
    std::string ontology;
    std::string interactionProtocol;
    std::string ownership;
};

class DFClient {
public:
    // Enregistre un service pour cet agent
    bool registerService(const std::string& agentName,
                         const std::string& type,
                         const std::string& serviceName,
                         const std::string& language            = "fipa-sl",
                         const std::string& ontology            = "-",
                         const std::string& interactionProtocol = "fipa-request",
                         const std::string& ownership           = "-");

    // Retire tous les services de cet agent
    bool deregisterAgent(const std::string& agentName);

    // Cherche par type de service
    std::vector<ServiceInfo> search(const std::string& type);

    // Cherche par type ET ontologie
    std::vector<ServiceInfo> search(const std::string& type,
                                    const std::string& ontology);

private:
    std::string request(const std::string& cmd);
    std::vector<ServiceInfo> parse_results(const std::string& response);
};

} // namespace platform
} // namespace gagent

#endif
