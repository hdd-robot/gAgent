/*
 * PlatformConfig.hpp — Chemins des sockets Unix de la plateforme gAgent
 *
 * Ces constantes sont partagées entre libgagent (clients) et
 * gAgentPlatform (serveurs). Elles peuvent être surchargées via
 * les variables d'environnement GAGENT_AMS_SOCK et GAGENT_DF_SOCK.
 */

#ifndef GAGENT_PLATFORM_CONFIG_HPP_
#define GAGENT_PLATFORM_CONFIG_HPP_

#include <cstdlib>

namespace gagent {
namespace platform {

inline const char* ams_socket_path() {
    const char* env = std::getenv("GAGENT_AMS_SOCK");
    return env ? env : "/tmp/gagent_ams.sock";
}

inline const char* df_socket_path() {
    const char* env = std::getenv("GAGENT_DF_SOCK");
    return env ? env : "/tmp/gagent_df.sock";
}

inline const char* env_socket_path() {
    const char* env = std::getenv("GAGENT_ENV_SOCK");
    return env ? env : "/tmp/gagent_env.sock";
}

} // namespace platform
} // namespace gagent

#endif
