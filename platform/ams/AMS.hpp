/*
 * AMS.hpp — Agent Management System
 *
 * Registre central des agents de la plateforme.
 * Conforme FIPA : chaque agent doit s'enregistrer auprès de l'AMS
 * avant de pouvoir communiquer.
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

class AMS {
public:
    // Enregistre un agent — retourne false si le nom est déjà pris
    bool registerAgent(const AgentRecord& rec);

    // Désenregistre un agent par nom
    bool deregisterAgent(const std::string& name);

    // Cherche un agent par nom
    std::optional<AgentRecord> lookup(const std::string& name) const;

    // Met à jour l'état d'un agent
    bool setState(const std::string& name, const std::string& state);

    // Affiche le registre (debug)
    void dump() const;

    std::size_t size() const;

    // Démarre le serveur socket Unix (boucle bloquante — à lancer dans un thread)
    void serve(const std::string& socket_path);

private:
    mutable std::mutex mutex_;
    std::map<std::string, AgentRecord> registry_;

    void handle_client(int fd);
};

} // namespace platform
} // namespace gagent

#endif
