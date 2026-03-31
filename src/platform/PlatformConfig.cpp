#include <gagent/platform/PlatformConfig.hpp>

#include <fstream>
#include <iostream>

namespace gagent {
namespace platform {

PlatformConfig& PlatformConfig::instance()
{
    static PlatformConfig inst;
    return inst;
}

PlatformConfig::PlatformConfig()
{
    load("/tmp/gagent.cfg");
}

void PlatformConfig::load(const std::string& path)
{
    std::ifstream f(path);
    if (!f) return;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key   = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if      (key == "master_ip")      master_ip_      = value;
        else if (key == "master_port")    master_port_    = std::stoi(value);
        else if (key == "slave_ip")       slave_ip_       = value;
        else if (key == "control_port")   control_port_   = std::stoi(value);
        else if (key == "base_port")      base_port_      = std::stoi(value);
        else if (key == "socket_timeout") socket_timeout_ = std::stoi(value);
    }
}

} // namespace platform
} // namespace gagent
