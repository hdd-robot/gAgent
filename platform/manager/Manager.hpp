/*
 * Manager.hpp — Interface de gestion des agents (agentmanager)
 *
 * Sous-commandes :
 *   agentmanager help
 *   agentmanager list
 *   agentmanager watch   [intervalle_ms]
 *   agentmanager kill    <nom>
 *   agentmanager suspend <nom>
 *   agentmanager wake    <nom>
 *   agentmanager df search <type> [ontologie]
 */

#ifndef GAGENT_PLATFORM_MANAGER_HPP_
#define GAGENT_PLATFORM_MANAGER_HPP_

#include <gagent/platform/AMSClient.hpp>
#include <gagent/platform/DFClient.hpp>
#include <string>
#include <vector>

namespace gagent {
namespace platform {

class Manager {
public:
    // Point d'entrée principal — dispatche sur la sous-commande
    int run(int argc, char* argv[]);

private:
    void cmd_help();
    void cmd_list();
    void cmd_watch(int interval_ms);
    void cmd_kill   (const std::string& name);
    void cmd_suspend(const std::string& name);
    void cmd_wake   (const std::string& name);
    void cmd_df_search(const std::string& type,
                       const std::string& ontology = "");

    // Envoie un signal RT à l'agent <name> (PID récupéré dans l'AMS)
    bool send_signal(const std::string& name, int sig);

    // Affichage formaté de la liste des agents
    void print_agents(const std::vector<AgentInfo>& agents);
    void print_services(const std::vector<ServiceInfo>& services);
};

} // namespace platform
} // namespace gagent

#endif
