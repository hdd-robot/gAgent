#include "AMS.hpp"
#include "../SlaveRegistry.hpp"
#include "../common/socket_utils.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <cerrno>

#include <sstream>
#include <cstring>
#include <thread>

namespace gagent {
namespace platform {

/* ------------------------------------------------------------------ */
/* Gestionnaire de connexion (partagé Unix + TCP)                      */
/* ------------------------------------------------------------------ */

void AMS::handle_client(int fd, const std::string& peer_ip)
{
    std::string line = readline_fd(fd);
    std::istringstream ss(line);
    std::string cmd;
    ss >> cmd;

    if (cmd == "REGISTER") {
        std::string name, address;
        int pid = -1;
        ss >> name >> pid >> address;
        // slave_ip optionnel dans la commande (priorité sur peer_ip)
        std::string slave_ip;
        ss >> slave_ip;
        if (slave_ip.empty() && !peer_ip.empty()) slave_ip = peer_ip;
        AgentRecord rec{name, address, pid, "active", slave_ip};
        write_str(fd, registerAgent(rec) ? "OK\n" : "ERROR already_registered\n");

    } else if (cmd == "DEREGISTER") {
        std::string name;
        ss >> name;
        write_str(fd, deregisterAgent(name) ? "OK\n" : "ERROR not_found\n");

    } else if (cmd == "LOOKUP") {
        std::string name;
        ss >> name;
        auto rec = lookup(name);
        // Pour les agents locaux, vérifier la vivacité via kill()
        if (rec && rec->slave_ip.empty()
                && ::kill(rec->pid, 0) < 0 && errno == ESRCH) {
            deregisterAgent(name);
            rec = std::nullopt;
        }
        if (rec) {
            std::string slave = rec->slave_ip.empty() ? "-" : rec->slave_ip;
            write_str(fd, "OK " + rec->name + " "
                      + std::to_string(rec->pid) + " "
                      + rec->address + " " + rec->state
                      + " " + slave + "\n");
        } else {
            write_str(fd, "ERROR not_found\n");
        }

    } else if (cmd == "SETSTATE") {
        std::string name, state;
        ss >> name >> state;
        write_str(fd, setState(name, state) ? "OK\n" : "ERROR not_found\n");

    } else if (cmd == "LIST") {
        // Purge les agents locaux morts
        {
            std::lock_guard<std::mutex> lk(mutex_);
            for (auto it = registry_.begin(); it != registry_.end(); ) {
                auto& r = it->second;
                if (r.slave_ip.empty()
                        && ::kill(r.pid, 0) < 0 && errno == ESRCH) {
                    std::cout << "[AMS] purge (mort) : " << r.name
                              << " (pid=" << r.pid << ")\n";
                    it = registry_.erase(it);
                } else {
                    ++it;
                }
            }
        }
        std::lock_guard<std::mutex> lk(mutex_);
        std::string resp = "OK " + std::to_string(registry_.size()) + "\n";
        for (auto& [n, r] : registry_) {
            std::string slave = r.slave_ip.empty() ? "-" : r.slave_ip;
            resp += r.name + " " + std::to_string(r.pid)
                  + " " + r.address + " " + r.state
                  + " " + slave + "\n";
        }
        write_str(fd, resp);

    } else if (cmd == "REGISTER_ENDPOINT") {
        // Upsert : crée ou met à jour l'endpoint de routage pour un nom visible
        std::string name, address;
        int pid = -1;
        ss >> name >> pid >> address;
        std::string slave_ip;
        ss >> slave_ip;
        if (slave_ip.empty() && !peer_ip.empty()) slave_ip = peer_ip;
        AgentRecord rec{name, address, pid, "active", slave_ip};
        registerEndpoint(rec);
        write_str(fd, "OK\n");

    } else if (cmd == "REGISTER_SLAVE") {
        // REGISTER_SLAVE <control_port>
        // peer_ip est l'IP de l'esclave (détectée via getpeername)
        int control_port = 40015;
        ss >> control_port;
        if (peer_ip.empty()) {
            write_str(fd, "ERROR no_peer_ip\n");
            return;
        }
        // Purge les anciens agents de cet esclave (reconnexion après crash)
        deregisterSlave(peer_ip);
        if (slave_registry_)
            slave_registry_->registerSlave(peer_ip, control_port);
        write_str(fd, "OK " + peer_ip + "\n");

    } else if (cmd == "HEARTBEAT") {
        std::string slave_ip;
        ss >> slave_ip;
        if (slave_registry_)
            slave_registry_->heartbeat(slave_ip);
        write_str(fd, "OK\n");

    } else if (cmd == "DEREGISTER_SLAVE") {
        std::string slave_ip;
        ss >> slave_ip;
        deregisterSlave(slave_ip);
        if (slave_registry_)
            slave_registry_->deregisterSlave(slave_ip);
        write_str(fd, "OK\n");

    } else if (cmd == "LIST_SLAVES") {
        if (!slave_registry_) {
            write_str(fd, "OK 0\n");
            return;
        }
        auto slaves = slave_registry_->list();
        std::string resp = "OK " + std::to_string(slaves.size()) + "\n";
        for (auto& [ip, port] : slaves)
            resp += ip + " " + std::to_string(port) + "\n";
        write_str(fd, resp);

    } else {
        write_str(fd, "ERROR unknown_command\n");
    }
}

/* ------------------------------------------------------------------ */
/* Serveur Unix                                                         */
/* ------------------------------------------------------------------ */

void AMS::serve(const std::string& socket_path)
{
    ::unlink(socket_path.c_str());

    int server_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[AMS] impossible de créer le socket Unix\n";
        return;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socket_path.c_str(),
                 sizeof(addr.sun_path) - 1);

    if (::bind(server_fd, reinterpret_cast<struct sockaddr*>(&addr),
               sizeof(addr)) < 0) {
        std::cerr << "[AMS] bind Unix échoué : " << socket_path << "\n";
        ::close(server_fd);
        return;
    }

    ::listen(server_fd, 32);
    std::cout << "[AMS] socket Unix prêt : " << socket_path << "\n";

    while (true) {
        int client_fd = ::accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;
        std::thread([this, client_fd]() {
            handle_client(client_fd, "");
            ::close(client_fd);
        }).detach();
    }
}

/* ------------------------------------------------------------------ */
/* Serveur TCP                                                          */
/* ------------------------------------------------------------------ */

void AMS::serve_tcp(int port)
{
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[AMS] impossible de créer le socket TCP\n";
        return;
    }

    int opt = 1;
    ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port));

    if (::bind(server_fd, reinterpret_cast<struct sockaddr*>(&addr),
               sizeof(addr)) < 0) {
        std::cerr << "[AMS] bind TCP échoué sur port " << port << "\n";
        ::close(server_fd);
        return;
    }

    ::listen(server_fd, 32);
    std::cout << "[AMS] socket TCP prêt sur port " << port << "\n";

    while (true) {
        struct sockaddr_in peer_addr{};
        socklen_t peer_len = sizeof(peer_addr);
        int client_fd = ::accept(server_fd,
                                  reinterpret_cast<struct sockaddr*>(&peer_addr),
                                  &peer_len);
        if (client_fd < 0) continue;

        char ip_buf[INET_ADDRSTRLEN];
        ::inet_ntop(AF_INET, &peer_addr.sin_addr, ip_buf, sizeof(ip_buf));
        std::string peer_ip = ip_buf;

        std::thread([this, client_fd, peer_ip]() {
            handle_client(client_fd, peer_ip);
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
              << " (pid=" << rec.pid;
    if (!rec.slave_ip.empty())
        std::cout << ", slave=" << rec.slave_ip;
    std::cout << ")\n";
    return true;
}

void AMS::registerEndpoint(const AgentRecord& rec)
{
    std::lock_guard<std::mutex> lk(mutex_);
    registry_[rec.name] = rec;   // crée ou écrase l'entrée
    std::cout << "[AMS] endpoint enregistré : " << rec.name
              << " → " << rec.address << "\n";
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

void AMS::deregisterSlave(const std::string& slave_ip)
{
    std::lock_guard<std::mutex> lk(mutex_);
    for (auto it = registry_.begin(); it != registry_.end(); ) {
        if (it->second.slave_ip == slave_ip) {
            std::cout << "[AMS] purge esclave " << slave_ip
                      << " : " << it->second.name << "\n";
            it = registry_.erase(it);
        } else {
            ++it;
        }
    }
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
                  << "  état=" << rec.state
                  << (rec.slave_ip.empty() ? "" : "  slave=" + rec.slave_ip)
                  << "\n";
}

std::size_t AMS::size() const
{
    std::lock_guard<std::mutex> lk(mutex_);
    return registry_.size();
}

} // namespace platform
} // namespace gagent
