#include "Manager.hpp"
#include <gagent/platform/PlatformConfig.hpp>

#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>

// Signaux RT partagés avec Agent.hpp
#define SIG_AGENT_DELETE  (__SIGRTMIN + 2)
#define SIG_AGENT_ACTIVE  (__SIGRTMIN + 3)
#define SIG_AGENT_SUSPEND (__SIGRTMIN + 4)
#define SIG_AGENT_WAKE    (__SIGRTMIN + 5)
#define SIG_AGENT_WAIT    (__SIGRTMIN + 6)

namespace gagent {
namespace platform {

/* ------------------------------------------------------------------ */
/* Affichage                                                            */
/* ------------------------------------------------------------------ */

static const char* state_color(const std::string& state)
{
    if (state == "active")    return "\033[32m";  // vert
    if (state == "suspended") return "\033[33m";  // jaune
    if (state == "waiting")   return "\033[36m";  // cyan
    if (state == "deleted")   return "\033[31m";  // rouge
    return "\033[0m";
}
static const char* RESET = "\033[0m";
static const char* BOLD  = "\033[1m";

void Manager::print_agents(const std::vector<AgentInfo>& agents)
{
    if (agents.empty()) {
        std::cout << "  (aucun agent enregistré)\n";
        return;
    }
    std::cout << BOLD
              << std::left
              << std::setw(20) << "NOM"
              << std::setw(8)  << "PID"
              << std::setw(30) << "ENDPOINT ZMQ"
              << std::setw(12) << "SLAVE"
              << "ÉTAT"
              << RESET << "\n";
    std::cout << std::string(74, '-') << "\n";
    for (auto& a : agents) {
        std::string slave = a.slave_ip.empty() ? "local" : a.slave_ip;
        std::cout << std::left
                  << std::setw(20) << a.name
                  << std::setw(8)  << a.pid
                  << std::setw(30) << a.address
                  << std::setw(12) << slave
                  << state_color(a.state) << a.state << RESET
                  << "\n";
    }
}

void Manager::print_services(const std::vector<ServiceInfo>& services)
{
    if (services.empty()) {
        std::cout << "  (aucun service trouvé)\n";
        return;
    }
    std::cout << BOLD
              << std::left
              << std::setw(20) << "AGENT"
              << std::setw(16) << "TYPE"
              << std::setw(20) << "SERVICE"
              << std::setw(12) << "LANGAGE"
              << "ONTOLOGIE"
              << RESET << "\n";
    std::cout << std::string(78, '-') << "\n";
    for (auto& s : services) {
        std::cout << std::left
                  << std::setw(20) << s.agentName
                  << std::setw(16) << s.type
                  << std::setw(20) << s.serviceName
                  << std::setw(12) << s.language
                  << s.ontology << "\n";
    }
}

/* ------------------------------------------------------------------ */
/* Commandes                                                            */
/* ------------------------------------------------------------------ */

void Manager::cmd_help()
{
    std::cout << BOLD << "agentmanager — gestionnaire gAgent\n" << RESET
              << "\n"
              << BOLD << "Usage :\n" << RESET
              << "  agentmanager <commande> [arguments]\n"
              << "\n"
              << BOLD << "Commandes agents :\n" << RESET
              << "  list                   Affiche les agents enregistrés dans l'AMS\n"
              << "  watch  [ms]            Affichage live (rafraîchi toutes les <ms> ms, défaut : 1000)\n"
              << "  kill   <nom>           Supprime l'agent (SIG_AGENT_DELETE)\n"
              << "  suspend <nom>          Suspend l'agent (SIG_AGENT_SUSPEND)\n"
              << "  wake   <nom>           Réveille l'agent (SIG_AGENT_WAKE)\n"
              << "\n"
              << BOLD << "Commandes DF :\n" << RESET
              << "  df search <type>             Cherche les services par type\n"
              << "  df search <type> <ontologie> Cherche avec filtre ontologie\n"
              << "\n"
              << BOLD << "Exemples :\n" << RESET
              << "  agentmanager list\n"
              << "  agentmanager watch 500\n"
              << "  agentmanager kill alice\n"
              << "  agentmanager df search planning\n"
              << "  agentmanager df search planning logistics\n"
              << "\n"
              << BOLD << "Prérequis :\n" << RESET
              << "  agentplatform doit être lancé (sockets "
              << ams_socket_path() << ")\n";
}

void Manager::cmd_list()
{
    AMSClient ams;
    auto agents = ams.list();
    std::cout << BOLD << "[AMS] " << agents.size() << " agent(s)\n" << RESET;
    print_agents(agents);
}

void Manager::cmd_watch(int interval_ms)
{
    while (true) {
        // Effacer l'écran + curseur en haut
        std::cout << "\033[2J\033[H";

        // Horodatage
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::cout << BOLD << "gAgent — " << std::put_time(std::localtime(&t), "%H:%M:%S")
                  << "  (Ctrl+C pour quitter)" << RESET << "\n\n";

        AMSClient ams;
        auto agents = ams.list();
        std::cout << BOLD << "Agents (" << agents.size() << ")\n" << RESET;
        print_agents(agents);

        std::cout << "\n";

        DFClient df;
        auto services = df.search("");   // liste tous (type vide = joker côté serveur)
        if (!services.empty()) {
            std::cout << BOLD << "Services DF (" << services.size() << ")\n" << RESET;
            print_services(services);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
}

static std::string ctrl_readline(int fd)
{
    std::string line;
    char c;
    while (::read(fd, &c, 1) == 1) {
        if (c == '\n') break;
        line += c;
    }
    return line;
}

/* Envoie une commande de contrôle à un esclave distant (KILL/SUSPEND/WAKE). */
static bool send_remote_signal(const std::string& slave_ip, int control_port,
                                int pid, const std::string& cmd_name)
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<uint16_t>(control_port));
    if (::inet_pton(AF_INET, slave_ip.c_str(), &addr.sin_addr) <= 0) {
        ::close(fd); return false;
    }

    struct timeval tv { 3, 0 };
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                  sizeof(addr)) < 0) {
        ::close(fd); return false;
    }

    std::string msg = cmd_name + " " + std::to_string(pid) + "\n";
    ::write(fd, msg.c_str(), msg.size());
    std::string resp = ctrl_readline(fd);
    ::close(fd);
    return resp.substr(0, 2) == "OK";
}

bool Manager::send_signal(const std::string& name, int sig)
{
    AMSClient ams;
    auto info = ams.lookup(name);
    if (!info) {
        std::cerr << "[manager] agent inconnu : " << name << "\n";
        return false;
    }

    // Agent distant ?
    if (!info->slave_ip.empty()) {
        // Trouver le port de contrôle de cet esclave
        auto slaves = ams.listSlaves();
        int ctrl_port = 40015;
        for (auto& [ip, port] : slaves)
            if (ip == info->slave_ip) { ctrl_port = port; break; }

        std::string cmd_name;
        if      (sig == SIG_AGENT_DELETE)  cmd_name = "KILL";
        else if (sig == SIG_AGENT_SUSPEND) cmd_name = "SUSPEND";
        else if (sig == SIG_AGENT_WAKE)    cmd_name = "WAKE";
        else { std::cerr << "[manager] signal inconnu\n"; return false; }

        if (!send_remote_signal(info->slave_ip, ctrl_port,
                                info->pid, cmd_name)) {
            std::cerr << "[manager] impossible de joindre le serveur de contrôle "
                      << info->slave_ip << ":" << ctrl_port << "\n";
            return false;
        }
        return true;
    }

    // Agent local
    union sigval sval;
    sval.sival_int = 0;
    if (sigqueue(info->pid, sig, sval) < 0) {
        perror("[manager] sigqueue");
        return false;
    }
    return true;
}

void Manager::cmd_kill(const std::string& name)
{
    if (send_signal(name, SIG_AGENT_DELETE))
        std::cout << "[manager] SIG_AGENT_DELETE → " << name << "\n";
}

void Manager::cmd_suspend(const std::string& name)
{
    if (send_signal(name, SIG_AGENT_SUSPEND)) {
        AMSClient ams;
        ams.setState(name, "suspended");
        std::cout << "[manager] SIG_AGENT_SUSPEND → " << name << "\n";
    }
}

void Manager::cmd_wake(const std::string& name)
{
    if (send_signal(name, SIG_AGENT_WAKE)) {
        AMSClient ams;
        ams.setState(name, "active");
        std::cout << "[manager] SIG_AGENT_WAKE → " << name << "\n";
    }
}

void Manager::cmd_df_search(const std::string& type,
                              const std::string& ontology)
{
    DFClient df;
    std::vector<ServiceInfo> services;
    if (ontology.empty())
        services = df.search(type);
    else
        services = df.search(type, ontology);

    std::cout << BOLD << "[DF] " << services.size()
              << " service(s) pour type=\"" << type << "\"";
    if (!ontology.empty()) std::cout << " ontologie=\"" << ontology << "\"";
    std::cout << "\n" << RESET;
    print_services(services);
}

/* ------------------------------------------------------------------ */
/* Dispatch                                                             */
/* ------------------------------------------------------------------ */

int Manager::run(int argc, char* argv[])
{
    // argv[0] = binaire, argv[1] = sous-commande
    if (argc < 2) {
        cmd_help();
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        cmd_help();

    } else if (cmd == "list") {
        cmd_list();

    } else if (cmd == "watch") {
        int ms = 1000;
        if (argc >= 3) {
            try { ms = std::stoi(argv[2]); }
            catch (...) { std::cerr << "intervalle invalide\n"; return 1; }
        }
        cmd_watch(ms);

    } else if (cmd == "kill") {
        if (argc < 3) { std::cerr << "usage : agentmanager kill <nom>\n"; return 1; }
        cmd_kill(argv[2]);

    } else if (cmd == "suspend") {
        if (argc < 3) { std::cerr << "usage : agentmanager suspend <nom>\n"; return 1; }
        cmd_suspend(argv[2]);

    } else if (cmd == "wake") {
        if (argc < 3) { std::cerr << "usage : agentmanager wake <nom>\n"; return 1; }
        cmd_wake(argv[2]);

    } else if (cmd == "df") {
        if (argc < 4 || std::string(argv[2]) != "search") {
            std::cerr << "usage : agentmanager df search <type> [ontologie]\n";
            return 1;
        }
        std::string type = argv[3];
        std::string ont  = (argc >= 5) ? argv[4] : "";
        cmd_df_search(type, ont);

    } else {
        std::cerr << "Commande inconnue : " << cmd
                  << "\nLancez 'agentmanager help' pour la liste des commandes.\n";
        return 1;
    }

    return 0;
}

} // namespace platform
} // namespace gagent
