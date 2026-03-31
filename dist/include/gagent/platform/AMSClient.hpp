/*
 * AMSClient.hpp — Client de l'Agent Management System
 *
 * En mode local : socket Unix.
 * En mode cluster : socket TCP vers le master.
 *
 * Usage type dans Agent::_init() :
 *   AMSClient ams;
 *   ams.registerAgent("alice", getpid(), endpoint);
 */

#ifndef GAGENT_PLATFORM_AMSCLIENT_HPP_
#define GAGENT_PLATFORM_AMSCLIENT_HPP_

#include <string>
#include <optional>
#include <vector>

namespace gagent {
namespace platform {

struct AgentInfo {
    std::string name;
    std::string address;   // endpoint ZMQ : ipc:///tmp/acl_<n> ou tcp://ip:port
    int         pid = -1;
    std::string state;     // active | suspended | waiting | deleted
    std::string slave_ip;  // vide = agent local
};

class AMSClient {
public:
    // Enregistre l'agent — retourne true si succès
    bool registerAgent(const std::string& name,
                       int               pid,
                       const std::string& endpoint);

    // Enregistre ou met à jour l'endpoint de routage (utilisé par acl_bind)
    // Toujours succède — crée ou écrase l'entrée existante
    void registerEndpoint(const std::string& name,
                          int               pid,
                          const std::string& endpoint);

    // Désenregistre l'agent
    bool deregisterAgent(const std::string& name);

    // Cherche un agent par nom
    std::optional<AgentInfo> lookup(const std::string& name);

    // Met à jour l'état (active | suspended | waiting | deleted)
    bool setState(const std::string& name, const std::string& state);

    // Liste tous les agents enregistrés
    std::vector<AgentInfo> list();

    // Liste les esclaves : retourne paires (slave_ip, control_port)
    std::vector<std::pair<std::string, int>> listSlaves();

private:
    // Ouvre une connexion, envoie cmd, lit une ligne de réponse, ferme
    std::string request(const std::string& cmd);
    // Comme request() mais ouvre la connexion et la retourne ouverte pour lire plusieurs lignes
    int  open_connection();
};

} // namespace platform
} // namespace gagent

#endif
