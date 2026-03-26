#include <gagent/platform/DFClient.hpp>
#include <gagent/platform/PlatformConfig.hpp>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sstream>
#include <iostream>
#include <cstring>

namespace gagent {
namespace platform {

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

/* Ouvre une connexion vers le DF (Unix ou TCP selon PlatformConfig). */
static int open_df_connection()
{
    auto& cfg = PlatformConfig::instance();

    if (cfg.isCluster() && !cfg.masterIP().empty()) {
        // Mode cluster : TCP vers le master (port DF = AMS+1)
        int df_port = cfg.masterPort() + 1;

        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return -1;

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(static_cast<uint16_t>(df_port));
        if (::inet_pton(AF_INET, cfg.masterIP().c_str(),
                        &addr.sin_addr) <= 0) {
            ::close(fd); return -1;
        }

        struct timeval tv { 2, 0 };
        ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                      sizeof(addr)) < 0) {
            ::close(fd); return -1;
        }
        return fd;
    } else {
        int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) return -1;

        struct sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, PlatformConfig::df_socket_path(),
                     sizeof(addr.sun_path) - 1);

        if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                      sizeof(addr)) < 0) {
            ::close(fd); return -1;
        }
        return fd;
    }
}

std::string DFClient::request(const std::string& cmd)
{
    int fd = open_df_connection();
    if (fd < 0) return "";

    std::string msg = cmd + "\n";
    ::write(fd, msg.c_str(), msg.size());

    std::string response = readline_fd(fd);
    ::close(fd);
    return response;
}

std::vector<ServiceInfo> DFClient::parse_results(const std::string& response)
{
    std::vector<ServiceInfo> result;
    std::istringstream ss(response);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        ServiceInfo s;
        ls >> s.agentName >> s.type >> s.serviceName
           >> s.language >> s.ontology
           >> s.interactionProtocol >> s.ownership;
        result.push_back(s);
    }
    return result;
}

/* ------------------------------------------------------------------ */

bool DFClient::registerService(const std::string& agentName,
                                const std::string& type,
                                const std::string& serviceName,
                                const std::string& language,
                                const std::string& ontology,
                                const std::string& interactionProtocol,
                                const std::string& ownership)
{
    auto& cfg = PlatformConfig::instance();
    // Slave IP explicite en mode cluster (sinon "-")
    std::string slave = cfg.isCluster() ? cfg.slaveIP() : "-";

    std::string cmd = "REGISTER " + agentName
                    + " " + std::to_string(::getpid())
                    + " " + type
                    + " " + serviceName
                    + " " + language
                    + " " + (ontology.empty() ? std::string("-") : ontology)
                    + " " + (interactionProtocol.empty() ? std::string("-") : interactionProtocol)
                    + " " + (ownership.empty() ? std::string("-") : ownership)
                    + " " + slave;

    std::string resp = request(cmd);
    if (resp.empty()) {
        std::cerr << "[DFClient] plateforme non disponible,"
                     " service non enregistré\n";
        return false;
    }
    return resp.substr(0, 2) == "OK";
}

bool DFClient::deregisterAgent(const std::string& agentName)
{
    std::string resp = request("DEREGISTER " + agentName);
    return !resp.empty() && resp.substr(0, 2) == "OK";
}

/* Pour SEARCH, la réponse est multi-lignes : on ouvre la connexion manuellement. */
static std::vector<ServiceInfo> search_impl(const std::string& cmd)
{
    std::vector<ServiceInfo> result;

    int fd = open_df_connection();
    if (fd < 0) return result;

    std::string msg = cmd + "\n";
    ::write(fd, msg.c_str(), msg.size());

    std::string first = readline_fd(fd);
    if (first.size() < 2 || first.substr(0, 2) != "OK") {
        ::close(fd); return result;
    }

    int count = std::stoi(first.substr(3));
    for (int i = 0; i < count; ++i) {
        std::string line = readline_fd(fd);
        std::istringstream ls(line);
        ServiceInfo s;
        ls >> s.agentName >> s.type >> s.serviceName
           >> s.language >> s.ontology
           >> s.interactionProtocol >> s.ownership;
        result.push_back(s);
    }
    ::close(fd);
    return result;
}

std::vector<ServiceInfo> DFClient::search(const std::string& type)
{
    return search_impl("SEARCH " + type);
}

std::vector<ServiceInfo> DFClient::search(const std::string& type,
                                           const std::string& ontology)
{
    return search_impl("SEARCH_ONT " + type + " " + ontology);
}

} // namespace platform
} // namespace gagent
