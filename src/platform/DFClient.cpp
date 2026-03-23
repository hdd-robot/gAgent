#include <gagent/platform/DFClient.hpp>
#include <gagent/platform/PlatformConfig.hpp>

#include <sys/socket.h>
#include <sys/un.h>
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

std::string DFClient::request(const std::string& cmd)
{
    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return "";

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, df_socket_path(),
                 sizeof(addr.sun_path) - 1);

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                  sizeof(addr)) < 0) {
        ::close(fd);
        return "";
    }

    std::string msg = cmd + "\n";
    ::write(fd, msg.c_str(), msg.size());

    std::string response = readline_fd(fd);
    ::close(fd);
    return response;
}

std::vector<ServiceInfo> DFClient::parse_results(const std::string& response)
{
    /* Utilisé pour SEARCH : la première ligne est "OK <count>",
     * lue séparément ; response contient les lignes de service
     * déjà concaténées. */
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
    std::string cmd = "REGISTER " + agentName
                    + " " + type
                    + " " + serviceName
                    + " " + language
                    + " " + ontology
                    + " " + interactionProtocol
                    + " " + ownership;
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

/* Pour SEARCH, la réponse est multi-lignes : on ouvre la connexion
 * manuellement pour lire plusieurs lignes. */
static std::vector<ServiceInfo> search_impl(const std::string& cmd)
{
    std::vector<ServiceInfo> result;

    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return result;

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, df_socket_path(),
                 sizeof(addr.sun_path) - 1);

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                  sizeof(addr)) < 0) {
        ::close(fd);
        return result;
    }

    std::string msg = cmd + "\n";
    ::write(fd, msg.c_str(), msg.size());

    std::string first = readline_fd(fd);
    if (first.substr(0, 2) != "OK") { ::close(fd); return result; }

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
