/*
 * Monitor.hpp — Serveur UDP de monitoring
 *
 * Reçoit et affiche les messages de log envoyés par les agents
 * via sendMsgMonitor() (UDP port 40013 par défaut).
 */

#ifndef GAGENT_PLATFORM_MONITOR_HPP_
#define GAGENT_PLATFORM_MONITOR_HPP_

#include <string>

namespace gagent {
namespace platform {

class Monitor {
public:
    Monitor(const std::string& ip, int port);
    void run();   // boucle bloquante

private:
    std::string ip_;
    int port_;
};

} // namespace platform
} // namespace gagent

#endif
