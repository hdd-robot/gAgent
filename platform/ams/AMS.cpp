#include "AMS.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <sstream>
#include <cstring>
#include <thread>

namespace gagent {
namespace platform {

/* ------------------------------------------------------------------ */
/* Serveur socket Unix                                                  */
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

static void write_str(int fd, const std::string& s)
{
    ::write(fd, s.c_str(), s.size());
}

void AMS::handle_client(int fd)
{
    std::string line = readline_fd(fd);
    std::istringstream ss(line);
    std::string cmd;
    ss >> cmd;

    if (cmd == "REGISTER") {
        std::string name, address;
        int pid = -1;
        ss >> name >> pid >> address;
        AgentRecord rec{name, address, pid, "active"};
        write_str(fd, registerAgent(rec) ? "OK\n" : "ERROR already_registered\n");

    } else if (cmd == "DEREGISTER") {
        std::string name;
        ss >> name;
        write_str(fd, deregisterAgent(name) ? "OK\n" : "ERROR not_found\n");

    } else if (cmd == "LOOKUP") {
        std::string name;
        ss >> name;
        auto rec = lookup(name);
        if (rec)
            write_str(fd, "OK " + rec->name + " "
                      + std::to_string(rec->pid) + " "
                      + rec->address + " " + rec->state + "\n");
        else
            write_str(fd, "ERROR not_found\n");

    } else if (cmd == "SETSTATE") {
        std::string name, state;
        ss >> name >> state;
        write_str(fd, setState(name, state) ? "OK\n" : "ERROR not_found\n");

    } else if (cmd == "LIST") {
        std::lock_guard<std::mutex> lk(mutex_);
        std::string resp = "OK " + std::to_string(registry_.size()) + "\n";
        for (auto& [n, r] : registry_)
            resp += r.name + " " + std::to_string(r.pid)
                  + " " + r.address + " " + r.state + "\n";
        write_str(fd, resp);

    } else {
        write_str(fd, "ERROR unknown_command\n");
    }
}

void AMS::serve(const std::string& socket_path)
{
    ::unlink(socket_path.c_str());

    int server_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[AMS] impossible de créer le socket\n";
        return;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socket_path.c_str(),
                 sizeof(addr.sun_path) - 1);

    if (::bind(server_fd, reinterpret_cast<struct sockaddr*>(&addr),
               sizeof(addr)) < 0) {
        std::cerr << "[AMS] bind échoué : " << socket_path << "\n";
        ::close(server_fd);
        return;
    }

    ::listen(server_fd, 32);
    std::cout << "[AMS] socket Unix prêt : " << socket_path << "\n";

    while (true) {
        int client_fd = ::accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;
        std::thread([this, client_fd]() {
            handle_client(client_fd);
            ::close(client_fd);
        }).detach();
    }
}

/* ------------------------------------------------------------------ */

bool AMS::registerAgent(const AgentRecord& rec)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (registry_.count(rec.name)) {
        std::cerr << "[AMS] nom déjà enregistré : " << rec.name << "\n";
        return false;
    }
    registry_[rec.name] = rec;
    std::cout << "[AMS] enregistré : " << rec.name
              << " (pid=" << rec.pid << ")\n";
    return true;
}

bool AMS::deregisterAgent(const std::string& name)
{
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = registry_.find(name);
    if (it == registry_.end()) return false;
    registry_.erase(it);
    std::cout << "[AMS] désenregistré : " << name << "\n";
    return true;
}

std::optional<AgentRecord> AMS::lookup(const std::string& name) const
{
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = registry_.find(name);
    if (it == registry_.end()) return std::nullopt;
    return it->second;
}

bool AMS::setState(const std::string& name, const std::string& state)
{
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = registry_.find(name);
    if (it == registry_.end()) return false;
    it->second.state = state;
    return true;
}

void AMS::dump() const
{
    std::lock_guard<std::mutex> lk(mutex_);
    std::cout << "[AMS] registre (" << registry_.size() << " agents) :\n";
    for (auto& [name, rec] : registry_)
        std::cout << "  " << name
                  << "  pid=" << rec.pid
                  << "  addr=" << rec.address
                  << "  état=" << rec.state << "\n";
}

std::size_t AMS::size() const
{
    std::lock_guard<std::mutex> lk(mutex_);
    return registry_.size();
}

} // namespace platform
} // namespace gagent
