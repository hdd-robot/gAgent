#include <gagent/platform/AMSClient.hpp>
#include <gagent/platform/PlatformConfig.hpp>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sstream>
#include <iostream>
#include <cerrno>
#include <cstring>

namespace gagent {
namespace platform {

/* ------------------------------------------------------------------ */
/* Utilitaires internes                                                 */
/* ------------------------------------------------------------------ */

static std::string readline_fd(int fd)
{
    std::string line;
    char c;
    while (::read(fd, &c, 1) == 1) {
        if (c == '\n') break;
        line += c;
    }
    return line;
}

/* Ouvre une connexion vers l'AMS (Unix ou TCP selon PlatformConfig).
 * Retourne le fd ouvert, ou -1 en cas d'erreur. */
int AMSClient::open_connection()
{
    auto& cfg = PlatformConfig::instance();

    if (cfg.isCluster() && !cfg.masterIP().empty()) {
        // Mode cluster : TCP vers le master
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return -1;

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(static_cast<uint16_t>(cfg.masterPort()));
        if (::inet_pton(AF_INET, cfg.masterIP().c_str(),
                        &addr.sin_addr) <= 0) {
            ::close(fd);
            return -1;
        }

        struct timeval tv { 2, 0 };  // timeout connexion 2 s
        ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                      sizeof(addr)) < 0) {
            ::close(fd);
            return -1;
        }
        return fd;
    } else {
        // Mode local : socket Unix
        int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) return -1;

        struct sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, PlatformConfig::ams_socket_path(),
                     sizeof(addr.sun_path) - 1);

        if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                      sizeof(addr)) < 0) {
            ::close(fd);
            return -1;
        }
        return fd;
    }
}

std::string AMSClient::request(const std::string& cmd)
{
    int fd = open_connection();
    if (fd < 0) return "";

    std::string msg = cmd + "\n";
    ::write(fd, msg.c_str(), msg.size());
    std::string response = readline_fd(fd);
    ::close(fd);
    return response;
}

/* ------------------------------------------------------------------ */

bool AMSClient::registerAgent(const std::string& name,
                               int               pid,
                               const std::string& endpoint)
{
    auto& cfg  = PlatformConfig::instance();
    std::string cmd = "REGISTER " + name + " "
                    + std::to_string(pid) + " " + endpoint;
    // En cluster, inclure explicitement l'IP esclave
    if (cfg.isCluster() && !cfg.slaveIP().empty())
        cmd += " " + cfg.slaveIP();

    std::string resp = request(cmd);
    if (resp.empty()) {
        std::cerr << "[AMSClient] plateforme non disponible,"
                     " agent " << name << " non enregistré\n";
        return false;
    }
    return resp.substr(0, 2) == "OK";
}

void AMSClient::registerEndpoint(const std::string& name,
                                  int               pid,
                                  const std::string& endpoint)
{
    auto& cfg  = PlatformConfig::instance();
    std::string cmd = "REGISTER_ENDPOINT " + name + " "
                    + std::to_string(pid) + " " + endpoint;
    if (cfg.isCluster() && !cfg.slaveIP().empty())
        cmd += " " + cfg.slaveIP();
    request(cmd);   // résultat ignoré
}

bool AMSClient::deregisterAgent(const std::string& name)
{
    std::string resp = request("DEREGISTER " + name);
    return !resp.empty() && resp.substr(0, 2) == "OK";
}

std::optional<AgentInfo> AMSClient::lookup(const std::string& name)
{
    std::string resp = request("LOOKUP " + name);
    if (resp.size() < 2 || resp.substr(0, 2) != "OK") return std::nullopt;

    // OK <name> <pid> <address> <state> <slave_ip>
    std::istringstream ss(resp.substr(3));
    AgentInfo info;
    std::string slave;
    ss >> info.name >> info.pid >> info.address >> info.state >> slave;
    if (slave != "-") info.slave_ip = slave;
    return info;
}

bool AMSClient::setState(const std::string& name,
                          const std::string& state)
{
    std::string resp = request("SETSTATE " + name + " " + state);
    return !resp.empty() && resp.substr(0, 2) == "OK";
}

std::vector<AgentInfo> AMSClient::list()
{
    std::vector<AgentInfo> result;

    int fd = open_connection();
    if (fd < 0) return result;

    std::string msg = "LIST\n";
    ::write(fd, msg.c_str(), msg.size());

    std::string first = readline_fd(fd);
    if (first.size() < 2 || first.substr(0, 2) != "OK") {
        ::close(fd); return result;
    }

    int count = std::stoi(first.substr(3));
    for (int i = 0; i < count; ++i) {
        std::string line = readline_fd(fd);
        std::istringstream ss(line);
        AgentInfo info;
        std::string slave;
        ss >> info.name >> info.pid >> info.address >> info.state >> slave;
        if (slave != "-") info.slave_ip = slave;
        result.push_back(info);
    }
    ::close(fd);
    return result;
}

std::vector<std::pair<std::string, int>> AMSClient::listSlaves()
{
    std::vector<std::pair<std::string, int>> result;

    int fd = open_connection();
    if (fd < 0) return result;

    std::string msg = "LIST_SLAVES\n";
    ::write(fd, msg.c_str(), msg.size());

    std::string first = readline_fd(fd);
    if (first.size() < 2 || first.substr(0, 2) != "OK") {
        ::close(fd); return result;
    }

    int count = std::stoi(first.substr(3));
    for (int i = 0; i < count; ++i) {
        std::string line = readline_fd(fd);
        std::istringstream ss(line);
        std::string ip;
        int port = 0;
        ss >> ip >> port;
        result.emplace_back(ip, port);
    }
    ::close(fd);
    return result;
}

} // namespace platform
} // namespace gagent
