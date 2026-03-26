/*
 * AMS.hpp — Agent Management System
 *
 * Registre central des agents de la plateforme.
 * Conforme FIPA : chaque agent doit s'enregistrer auprès de l'AMS
 * avant de pouvoir communiquer.
 *
 * Deux serveurs :
 *   serve()     — socket Unix (agents locaux)
 *   serve_tcp() — socket TCP  (agents sur esclaves distants)
 */

#ifndef GAGENT_PLATFORM_AMS_HPP_
#define GAGENT_PLATFORM_AMS_HPP_

#include "AgentRecord.hpp"
#include <map>
#include <string>
#include <optional>
#include <mutex>
#include <iostream>
#include <thread>

namespace gagent {
namespace platform {

class SlaveRegistry;

class AMS {
public:
    // Enregistre un agent — retourne false si le nom est déjà pris
    bool registerAgent(const AgentRecord& rec);

    // Enregistre ou met à jour l'endpoint de routage d'un agent
    // (utilisé par acl_bind pour associer le nom visible à son endpoint ZMQ)
    void registerEndpoint(const AgentRecord& rec);

    // Désenregistre un agent par nom
    bool deregisterAgent(const std::string& name);

    // Purge tous les agents d'un esclave donné
    void deregisterSlave(const std::string& slave_ip);

    // Cherche un agent par nom
    std::optional<AgentRecord> lookup(const std::string& name) const;

    // Met à jour l'état d'un agent
    bool setState(const std::string& name, const std::string& state);

    // Affiche le registre (debug)
    void dump() const;

    std::size_t size() const;

    // Associe le registre des esclaves (pour les commandes REGISTER_SLAVE, etc.)
    void setSlaveRegistry(SlaveRegistry* reg) { slave_registry_ = reg; }

    // Démarre le serveur socket Unix (boucle bloquante — à lancer dans un thread)
    void serve(const std::string& socket_path);

    // Démarre le serveur TCP (boucle bloquante — à lancer dans un thread)
    void serve_tcp(int port);

private:
    mutable std::mutex mutex_;
    std::map<std::string, AgentRecord> registry_;
    SlaveRegistry* slave_registry_ = nullptr;

    void handle_client(int fd, const std::string& peer_ip = "");
};

} // namespace platform
} // namespace gagent

#endif
