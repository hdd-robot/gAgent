#include <gagent/platform/EnvClient.hpp>
#include <gagent/platform/PlatformConfig.hpp>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

namespace gagent {
namespace platform {

std::string EnvClient::request(const std::string& cmd)
{
    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return "";

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, env_socket_path(), sizeof(addr.sun_path) - 1);

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return "";
    }

    std::string msg = cmd + "\n";
    ::write(fd, msg.c_str(), msg.size());

    // Read response until server closes connection
    std::string response;
    char buf[4096];
    ssize_t n;
    while ((n = ::read(fd, buf, sizeof(buf))) > 0)
        response.append(buf, n);
    ::close(fd);

    // Strip trailing newline
    while (!response.empty() && (response.back() == '\n' || response.back() == '\r'))
        response.pop_back();

    return response;
}

std::string EnvClient::getAgents() { return request("GET_AGENTS"); }
std::string EnvClient::getNsap()   { return request("GET_NSAP"); }

} // namespace platform
} // namespace gagent
