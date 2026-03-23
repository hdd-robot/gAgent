#include <gagent/platform/AMSClient.hpp>
#include <gagent/platform/PlatformConfig.hpp>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <sstream>
#include <iostream>
#include <cerrno>
#include <cstring>

namespace gagent {
namespace platform {

/* ------------------------------------------------------------------ */
/* Utilitaire interne : connexion → envoi → lecture réponse → fermeture */
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

std::string AMSClient::request(const std::string& cmd)
{
    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return "";

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, ams_socket_path(),
                 sizeof(addr.sun_path) - 1);

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                  sizeof(addr)) < 0) {
        ::close(fd);
        return "";   // plateforme non disponible — mode dégradé
    }

    std::string msg = cmd + "\n";
    ::write(fd, msg.c_str(), msg.size());

    std::string response = readline_fd(fd);
    ::close(fd);
    return response;
}

/* ------------------------------------------------------------------ */

bool AMSClient::registerAgent(const std::string& name,
                               int               pid,
                               const std::string& mq_address)
{
    std::string cmd = "REGISTER " + name + " "
                    + std::to_string(pid) + " " + mq_address;
    std::string resp = request(cmd);
    if (resp.empty()) {
        std::cerr << "[AMSClient] plateforme non disponible,"
                     " agent " << name << " non enregistré\n";
        return false;
    }
    return resp.substr(0, 2) == "OK";
}

bool AMSClient::deregisterAgent(const std::string& name)
{
    std::string resp = request("DEREGISTER " + name);
    return resp.substr(0, 2) == "OK";
}

std::optional<AgentInfo> AMSClient::lookup(const std::string& name)
{
    std::string resp = request("LOOKUP " + name);
    if (resp.substr(0, 2) != "OK") return std::nullopt;

    // OK <name> <pid> <address> <state>
    std::istringstream ss(resp.substr(3));
    AgentInfo info;
    ss >> info.name >> info.pid >> info.address >> info.state;
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
    /* Pour LIST, la réponse tient sur plusieurs lignes.
     * On ouvre la connexion manuellement. */
    std::vector<AgentInfo> result;

    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return result;

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, ams_socket_path(),
                 sizeof(addr.sun_path) - 1);

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                  sizeof(addr)) < 0) {
        ::close(fd);
        return result;
    }

    std::string msg = "LIST\n";
    ::write(fd, msg.c_str(), msg.size());

    std::string first = readline_fd(fd);
    if (first.substr(0, 2) != "OK") { ::close(fd); return result; }

    int count = std::stoi(first.substr(3));
    for (int i = 0; i < count; ++i) {
        std::string line = readline_fd(fd);
        std::istringstream ss(line);
        AgentInfo info;
        ss >> info.name >> info.pid >> info.address >> info.state;
        result.push_back(info);
    }
    ::close(fd);
    return result;
}

} // namespace platform
} // namespace gagent
