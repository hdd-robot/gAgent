/*
 * PlatformConfig.hpp — Configuration de la plateforme gAgent
 *
 * Singleton qui lit /tmp/gagent.cfg (écrit par agentplatform).
 * En mode local (aucun fichier), se comporte comme avant.
 *
 * Format du fichier de configuration (key=value) :
 *   master_ip=192.168.1.10
 *   master_port=40011
 *   slave_ip=192.168.1.20
 *   control_port=40015
 *   base_port=50000
 */

#ifndef GAGENT_PLATFORM_CONFIG_HPP_
#define GAGENT_PLATFORM_CONFIG_HPP_

#include <string>

namespace gagent {
namespace platform {

class PlatformConfig {
public:
    static PlatformConfig& instance();

    // IP du master AMS (vide en mode local pur)
    const std::string& masterIP()    const { return master_ip_; }
    int                masterPort()  const { return master_port_; }

    // IP de cette machine (vide si pas en cluster)
    const std::string& slaveIP()     const { return slave_ip_; }

    // Port du serveur de contrôle sur l'esclave (défaut 40015)
    int                controlPort() const { return control_port_; }

    // Port de base pour les endpoints ZMQ TCP (défaut 50000)
    int                basePort()    const { return base_port_; }

    // true si cette machine fait partie d'un cluster (master ou slave)
    bool               isCluster()   const { return !slave_ip_.empty(); }

    // Chemins des sockets Unix (inchangés, pour la compatibilité locale)
    static const char* ams_socket_path() { return "/tmp/gagent_ams.sock"; }
    static const char* df_socket_path()  { return "/tmp/gagent_df.sock";  }
    static const char* env_socket_path() { return "/tmp/gagent_env.sock"; }

private:
    PlatformConfig();   // lit /tmp/gagent.cfg
    void load(const std::string& path);

    std::string master_ip_;
    int         master_port_  = 40011;
    std::string slave_ip_;
    int         control_port_ = 40015;
    int         base_port_    = 50000;
};

// Fonctions libres pour la compatibilité avec l'ancien code
inline const char* ams_socket_path() { return PlatformConfig::ams_socket_path(); }
inline const char* df_socket_path()  { return PlatformConfig::df_socket_path();  }
inline const char* env_socket_path() { return PlatformConfig::env_socket_path(); }

} // namespace platform
} // namespace gagent

#endif
