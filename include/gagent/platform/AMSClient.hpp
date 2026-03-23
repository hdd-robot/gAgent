/*
 * AMSClient.hpp — Client de l'Agent Management System
 *
 * Permet à un agent de s'enregistrer, se désenregistrer et
 * interroger le registre AMS via socket Unix.
 *
 * Usage type dans Agent::_init() :
 *   AMSClient ams;
 *   ams.registerAgent("alice", getpid(), "/acl_alice");
 *
 * Si la plateforme n'est pas lancée, les opérations échouent
 * silencieusement (mode dégradé mono-nœud).
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
    std::string address;   // MQ name ex: /acl_alice
    int         pid = -1;
    std::string state;     // active | suspended | waiting | deleted
};

class AMSClient {
public:
    // Enregistre l'agent — retourne true si succès
    bool registerAgent(const std::string& name,
                       int               pid,
                       const std::string& mq_address);

    // Désenregistre l'agent
    bool deregisterAgent(const std::string& name);

    // Cherche un agent par nom
    std::optional<AgentInfo> lookup(const std::string& name);

    // Met à jour l'état (active | suspended | waiting | deleted)
    bool setState(const std::string& name, const std::string& state);

    // Liste tous les agents enregistrés
    std::vector<AgentInfo> list();

private:
    // Ouvre une connexion, envoie cmd, lit la réponse, ferme
    std::string request(const std::string& cmd);
};

} // namespace platform
} // namespace gagent

#endif
