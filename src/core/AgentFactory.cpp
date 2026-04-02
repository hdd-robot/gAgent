/*
 * AgentFactory.cpp — Fabrique d'agents et serveur de migration
 */

#include "AgentFactory.hpp"
#include "Agent.hpp"
#include <gagent/platform/PlatformConfig.hpp>
#include <gagent/utils/Logger.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <cstring>

namespace gagent {

// ── Singleton ─────────────────────────────────────────────────────────────────

AgentFactory& AgentFactory::instance() {
    static AgentFactory inst;
    return inst;
}

void AgentFactory::registerType(const std::string& type_name, Creator creator) {
    registry_[type_name] = std::move(creator);
}

Agent* AgentFactory::create(const std::string& type_name) const {
    auto it = registry_.find(type_name);
    if (it == registry_.end()) return nullptr;
    return it->second();
}

// ── Serveur de migration ──────────────────────────────────────────────────────

static std::string mig_readline(int fd)
{
    std::string line;
    char buf[4096];
    while (true) {
        ssize_t n = ::recv(fd, buf, sizeof(buf), MSG_PEEK);
        if (n <= 0) break;
        char* nl = static_cast<char*>(memchr(buf, '\n', static_cast<size_t>(n)));
        size_t to_read = nl ? static_cast<size_t>(nl - buf + 1)
                            : static_cast<size_t>(n);
        ssize_t r = ::read(fd, buf, to_read);
        if (r <= 0) break;
        if (nl) { line.append(buf, static_cast<size_t>(r) - 1); break; }
        line.append(buf, static_cast<size_t>(r));
    }
    return line;
}

static void handle_migration(int cli_fd, AgentFactory& factory)
{
    std::string line = mig_readline(cli_fd);
    if (!line.empty() && line.back() == '\r') line.pop_back();

    // Format : ARRIVE <type> <name> <attrs>
    std::istringstream ss(line);
    std::string cmd, type_name, agent_name, attrs_str;
    ss >> cmd >> type_name >> agent_name >> attrs_str;

    if (cmd != "ARRIVE" || type_name.empty() || agent_name.empty()) {
        const char* err = "ERR bad format\n";
        (void)::write(cli_fd, err, std::strlen(err));
        ::close(cli_fd);
        return;
    }

    Agent* ag = factory.create(type_name);
    if (!ag) {
        std::string err = "ERR unknown type " + type_name + "\n";
        (void)::write(cli_fd, err.c_str(), err.size());
        ::close(cli_fd);
        return;
    }

    ag->setAgentName(agent_name);

    // Restaurer les attributs (format: key:val;key:val;)
    if (attrs_str != "-") {
        std::istringstream as(attrs_str);
        std::string token;
        while (std::getline(as, token, ';')) {
            auto sep = token.find(':');
            if (sep == std::string::npos) continue;
            std::string k = token.substr(0, sep);
            std::string v = token.substr(sep + 1);
            ag->addAttribut(k);
            ag->setAttribut(k, v);
        }
    }

    (void)::write(cli_fd, "OK\n", 3);
    ::close(cli_fd);

    LOG_JSON("agent_arrived",
        {"agent", agent_name},
        {"type",  type_name}
    );

    // Démarre l'agent (fork interne)
    ag->init();
}

void AgentFactory::startMigrationServer(int port)
{
    if (port < 0)
        port = gagent::platform::PlatformConfig::instance().migrationPort();

    int srv = ::socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("migration socket"); return; }

    int opt = 1;
    ::setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port));

    if (::bind(srv, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("migration bind"); ::close(srv); return;
    }
    ::listen(srv, 8);
    std::cout << "[AgentFactory] serveur de migration prêt sur port " << port << "\n";

    while (true) {
        int cli = ::accept(srv, nullptr, nullptr);
        if (cli < 0) continue;
        handle_migration(cli, *this);
    }
}

} // namespace gagent
