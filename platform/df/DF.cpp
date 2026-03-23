#include "DF.hpp"
#include <algorithm>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <cerrno>
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

static std::string svc_to_line(const Service& s)
{
    auto tok = [](const std::string& v) {
        return v.empty() ? "-" : v;
    };
    return tok(s.agentName) + " " + tok(s.type) + " " + tok(s.name)
         + " " + tok(s.language) + " " + tok(s.ontology)
         + " " + tok(s.interactionProtocol) + " " + tok(s.ownership)
         + "\n";
}

void DF::handle_client(int fd)
{
    std::string line = readline_fd(fd);
    std::istringstream ss(line);
    std::string cmd;
    ss >> cmd;

    if (cmd == "REGISTER") {
        Service svc;
        ss >> svc.agentName >> svc.pid >> svc.type >> svc.name
           >> svc.language >> svc.ontology
           >> svc.interactionProtocol >> svc.ownership;
        // "-" means empty
        if (svc.ontology            == "-") svc.ontology            = "";
        if (svc.interactionProtocol == "-") svc.interactionProtocol = "";
        if (svc.ownership           == "-") svc.ownership           = "";
        registerService(svc);
        write_str(fd, "OK\n");

    } else if (cmd == "DEREGISTER") {
        std::string agentName;
        ss >> agentName;
        deregisterAgent(agentName);
        write_str(fd, "OK\n");

    } else if (cmd == "SEARCH" || cmd == "SEARCH_ONT") {
        // Purge les services dont le PID n'existe plus
        {
            std::lock_guard<std::mutex> lk(mutex_);
            services_.erase(
                std::remove_if(services_.begin(), services_.end(),
                    [](const Service& s) {
                        return s.pid > 0
                            && ::kill(s.pid, 0) < 0
                            && errno == ESRCH;
                    }),
                services_.end());
        }
        std::string type, ontology;
        ss >> type;
        std::vector<Service> found;
        if (cmd == "SEARCH_ONT") {
            ss >> ontology;
            found = search(type, ontology);
        } else if (type.empty() || type == "*") {
            std::lock_guard<std::mutex> lk(mutex_);
            found = services_;
        } else {
            found = search(type);
        }
        std::string resp = "OK " + std::to_string(found.size()) + "\n";
        for (auto& s : found) resp += svc_to_line(s);
        write_str(fd, resp);

    } else {
        write_str(fd, "ERROR unknown_command\n");
    }
}

void DF::serve(const std::string& socket_path)
{
    ::unlink(socket_path.c_str());

    int server_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[DF] impossible de créer le socket\n";
        return;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socket_path.c_str(),
                 sizeof(addr.sun_path) - 1);

    if (::bind(server_fd, reinterpret_cast<struct sockaddr*>(&addr),
               sizeof(addr)) < 0) {
        std::cerr << "[DF] bind échoué : " << socket_path << "\n";
        ::close(server_fd);
        return;
    }

    ::listen(server_fd, 32);
    std::cout << "[DF] socket Unix prêt : " << socket_path << "\n";

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

void DF::registerService(const Service& svc)
{
    std::lock_guard<std::mutex> lk(mutex_);
    services_.push_back(svc);
    std::cout << "[DF] enregistré : " << svc.name
              << " (type=" << svc.type
              << ", agent=" << svc.agentName << ")\n";
}

void DF::deregisterAgent(const std::string& agentName)
{
    std::lock_guard<std::mutex> lk(mutex_);
    auto before = services_.size();
    services_.erase(
        std::remove_if(services_.begin(), services_.end(),
            [&](const Service& s){ return s.agentName == agentName; }),
        services_.end());
    std::cout << "[DF] retiré " << (before - services_.size())
              << " service(s) de " << agentName << "\n";
}

std::vector<Service> DF::search(const std::string& type) const
{
    std::lock_guard<std::mutex> lk(mutex_);
    std::vector<Service> result;
    for (auto& s : services_)
        if (s.type == type) result.push_back(s);
    return result;
}

std::vector<Service> DF::search(const std::string& type,
                                const std::string& ontology) const
{
    std::lock_guard<std::mutex> lk(mutex_);
    std::vector<Service> result;
    for (auto& s : services_)
        if (s.type == type && s.ontology == ontology)
            result.push_back(s);
    return result;
}

void DF::dump() const
{
    std::lock_guard<std::mutex> lk(mutex_);
    std::cout << "[DF] annuaire (" << services_.size() << " service(s)) :\n";
    for (auto& s : services_)
        std::cout << "  " << s.name
                  << "  type=" << s.type
                  << "  agent=" << s.agentName
                  << "  ontologie=" << s.ontology << "\n";
}

} // namespace platform
} // namespace gagent
