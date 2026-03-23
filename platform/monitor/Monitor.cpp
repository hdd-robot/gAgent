#include "Monitor.hpp"
#include <gagent/utils/udp_client_server.hpp>
#include <iostream>
#include <iomanip>
#include <cstdio>

namespace gagent {
namespace platform {

static constexpr int BUFLEN = 1024;

Monitor::Monitor(const std::string& ip, int port)
    : ip_(ip), port_(port) {}

void Monitor::run()
{
    std::cout << "[Monitor] écoute sur " << ip_ << ":" << port_ << "\n";
    udp_client_server::udp_server server(ip_, port_);

    char msg[BUFLEN];
    int  num = 0;
    while (true) {
        server.recv(msg, BUFLEN);
        std::printf("%08d : %s\n", ++num, msg);
        std::fflush(stdout);
    }
}

} // namespace platform
} // namespace gagent
