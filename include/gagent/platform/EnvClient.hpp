/*
 * EnvClient.hpp — Client pour le socket de l'Environnement gAgent
 *
 * Protocole (ligne par commande) :
 *   GET_AGENTS  → JSON {"width":N,"height":N,"agents":[...]}
 *   GET_NSAP    → JSON {"count":N,"snaps":[{"seq":N,"timestamp":"..."},...]}
 */

#ifndef GAGENT_ENVCLIENT_HPP_
#define GAGENT_ENVCLIENT_HPP_

#include <string>

namespace gagent {
namespace platform {

class EnvClient {
public:
    /** Retourne le JSON des agents visuels courants, "" si env non disponible. */
    std::string getAgents();

    /** Retourne le JSON de la pile NSAP, "" si env non disponible. */
    std::string getNsap();

private:
    std::string request(const std::string& cmd);
};

} // namespace platform
} // namespace gagent

#endif /* GAGENT_ENVCLIENT_HPP_ */
