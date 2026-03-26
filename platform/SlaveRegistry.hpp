/*
 * SlaveRegistry.hpp — Registre des plateformes esclaves
 *
 * Tenu par le master : maintient la liste des esclaves connectés,
 * met à jour les heartbeats, et purge les agents/services des
 * esclaves morts.
 */

#ifndef GAGENT_PLATFORM_SLAVE_REGISTRY_HPP_
#define GAGENT_PLATFORM_SLAVE_REGISTRY_HPP_

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>

namespace gagent {
namespace platform {

class AMS;
class DF;

class SlaveRegistry {
public:
    struct SlaveInfo {
        int control_port;
        std::chrono::steady_clock::time_point last_heartbeat;
    };

    // Enregistre ou met à jour un esclave
    void registerSlave(const std::string& slave_ip, int control_port);

    // Met à jour le timestamp de heartbeat
    void heartbeat(const std::string& slave_ip);

    // Supprime un esclave proprement (déconnexion volontaire)
    void deregisterSlave(const std::string& slave_ip);

    // Cherche le port de contrôle d'un esclave
    int controlPort(const std::string& slave_ip) const;

    // Liste tous les esclaves actifs
    std::vector<std::pair<std::string, int>> list() const;

    // Démarre le thread watchdog (appelle ams/df.deregisterSlave sur timeout)
    void startWatchdog(AMS& ams, DF& df,
                       int timeout_s = 15, int check_interval_s = 5);

private:
    mutable std::mutex mutex_;
    std::map<std::string, SlaveInfo> slaves_;
};

} // namespace platform
} // namespace gagent

#endif
