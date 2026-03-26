/*
 * DF.hpp — Directory Facilitator
 *
 * Annuaire des services : les agents s'y enregistrent pour que
 * d'autres agents puissent les découvrir par type/ontologie.
 */

#ifndef GAGENT_PLATFORM_DF_HPP_
#define GAGENT_PLATFORM_DF_HPP_

#include "Service.hpp"
#include <vector>
#include <string>
#include <mutex>
#include <iostream>
#include <thread>

namespace gagent {
namespace platform {

class DF {
public:
    // Enregistre un service
    void registerService(const Service& svc);

    // Retire tous les services d'un agent
    void deregisterAgent(const std::string& agentName);

    // Recherche par type de service
    std::vector<Service> search(const std::string& type) const;

    // Recherche par type ET ontologie
    std::vector<Service> search(const std::string& type,
                                const std::string& ontology) const;

    // Retire tous les services des agents d'un esclave
    void deregisterSlave(const std::string& slave_ip);

    void dump() const;

    // Démarre le serveur socket Unix (boucle bloquante — à lancer dans un thread)
    void serve(const std::string& socket_path);

    // Démarre le serveur TCP (boucle bloquante — à lancer dans un thread)
    void serve_tcp(int port);

private:
    mutable std::mutex mutex_;
    std::vector<Service> services_;

    void handle_client(int fd, const std::string& peer_ip = "");
};

} // namespace platform
} // namespace gagent

#endif
