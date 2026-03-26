#include "SlaveRegistry.hpp"
#include "ams/AMS.hpp"
#include "df/DF.hpp"

#include <iostream>
#include <thread>

namespace gagent {
namespace platform {

void SlaveRegistry::registerSlave(const std::string& slave_ip, int control_port)
{
    std::lock_guard<std::mutex> lk(mutex_);
    slaves_[slave_ip] = { control_port,
                          std::chrono::steady_clock::now() };
    std::cout << "[SlaveRegistry] esclave enregistré : "
              << slave_ip << " (control_port=" << control_port << ")\n";
}

void SlaveRegistry::heartbeat(const std::string& slave_ip)
{
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = slaves_.find(slave_ip);
    if (it != slaves_.end())
        it->second.last_heartbeat = std::chrono::steady_clock::now();
}

void SlaveRegistry::deregisterSlave(const std::string& slave_ip)
{
    std::lock_guard<std::mutex> lk(mutex_);
    slaves_.erase(slave_ip);
    std::cout << "[SlaveRegistry] esclave supprimé : " << slave_ip << "\n";
}

int SlaveRegistry::controlPort(const std::string& slave_ip) const
{
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = slaves_.find(slave_ip);
    return (it != slaves_.end()) ? it->second.control_port : -1;
}

std::vector<std::pair<std::string, int>> SlaveRegistry::list() const
{
    std::lock_guard<std::mutex> lk(mutex_);
    std::vector<std::pair<std::string, int>> result;
    for (auto& [ip, info] : slaves_)
        result.emplace_back(ip, info.control_port);
    return result;
}

void SlaveRegistry::startWatchdog(AMS& ams, DF& df,
                                   int timeout_s, int check_interval_s)
{
    std::thread([this, &ams, &df, timeout_s, check_interval_s]() {
        while (true) {
            std::this_thread::sleep_for(
                std::chrono::seconds(check_interval_s));

            auto now  = std::chrono::steady_clock::now();
            auto limit = std::chrono::seconds(timeout_s);

            std::vector<std::string> dead;
            {
                std::lock_guard<std::mutex> lk(mutex_);
                for (auto& [ip, info] : slaves_)
                    if (now - info.last_heartbeat > limit)
                        dead.push_back(ip);
            }

            for (auto& ip : dead) {
                std::cout << "[SlaveRegistry] timeout esclave " << ip
                          << " — purge AMS+DF\n";
                ams.deregisterSlave(ip);
                df.deregisterSlave(ip);
                std::lock_guard<std::mutex> lk(mutex_);
                slaves_.erase(ip);
            }
        }
    }).detach();
}

} // namespace platform
} // namespace gagent
