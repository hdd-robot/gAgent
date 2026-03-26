/*
 * AgentRecord.hpp — Entrée dans le registre AMS
 *
 * L'AMS (Agent Management System) maintient un registre de tous les agents
 * actifs sur la plateforme : nom, PID, adresse, état.
 */

#ifndef GAGENT_PLATFORM_AGENTRECORD_HPP_
#define GAGENT_PLATFORM_AGENTRECORD_HPP_

#include <string>

namespace gagent {
namespace platform {

struct AgentRecord {
    std::string name;       // nom unique de l'agent
    std::string address;    // endpoint ZMQ : ipc:///tmp/acl_<name> ou tcp://ip:port
    int         pid = -1;   // PID du processus child
    std::string state;      // active | suspended | waiting | deleted
    std::string slave_ip;   // IP de la plateforme esclave (vide = local)
};

} // namespace platform
} // namespace gagent

#endif
