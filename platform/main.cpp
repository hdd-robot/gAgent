/*
 * main.cpp — Point d'entrée du daemon gAgent Platform
 *
 * Le binaire détecte son rôle via son nom (ou via un symlink) :
 *
 *   agentplatform  →  AMS + DF  (registre des agents et services)
 *   agentmanager   →  gestionnaire de plateforme
 *   agentmonitor   →  affichage des logs UDP des agents
 *
 * Modes agentplatform :
 *   ./agentplatform                           — local pur (sockets Unix)
 *   ./agentplatform --master --ip 192.168.1.10 — master du cluster
 *   ./agentplatform --slave 192.168.1.10:40011 [--ip override]
 */

#include "common/Args.hpp"
#include "ams/AMS.hpp"
#include "df/DF.hpp"
#include "monitor/Monitor.hpp"
#include "manager/Manager.hpp"
#include "SlaveRegistry.hpp"

#include <gagent/platform/PlatformConfig.hpp>

#include <libconfig.h++>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <csignal>
#include <cstring>
#include <atomic>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

// ── Détection automatique d'IP ────────────────────────────────────────────────

#include <ifaddrs.h>
#include <net/if.h>

static std::string detect_local_ip()
{
    struct ifaddrs* ifa = nullptr;
    ::getifaddrs(&ifa);
    std::string result;
    for (auto* p = ifa; p; p = p->ifa_next) {
        if (!p->ifa_addr) continue;
        if (p->ifa_addr->sa_family != AF_INET) continue;
        if (p->ifa_flags & IFF_LOOPBACK) continue;
        char buf[INET_ADDRSTRLEN];
        auto* sin = reinterpret_cast<struct sockaddr_in*>(p->ifa_addr);
        ::inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf));
        result = buf;
        break;
    }
    if (ifa) ::freeifaddrs(ifa);
    return result.empty() ? "127.0.0.1" : result;
}

// ── Écriture du fichier de configuration ─────────────────────────────────────

static void write_config(const std::string& master_ip, int master_port,
                          const std::string& slave_ip,  int control_port,
                          int base_port)
{
    std::ofstream f("/tmp/gagent.cfg");
    if (!f) { std::cerr << "[platform] impossible d'écrire /tmp/gagent.cfg\n"; return; }
    if (!master_ip.empty()) f << "master_ip="    << master_ip    << "\n";
    f << "master_port=" << master_port << "\n";
    if (!slave_ip.empty())  f << "slave_ip="     << slave_ip     << "\n";
    f << "control_port=" << control_port << "\n";
    f << "base_port="    << base_port    << "\n";
    std::cout << "[platform] configuration écrite dans /tmp/gagent.cfg\n";
}

// ── Serveur de contrôle esclave ───────────────────────────────────────────────

#define SIG_AGENT_DELETE  (__SIGRTMIN + 2)
#define SIG_AGENT_SUSPEND (__SIGRTMIN + 4)
#define SIG_AGENT_WAKE    (__SIGRTMIN + 5)

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

static void handle_control(int client_fd)
{
    std::string line = readline_fd(client_fd);
    std::istringstream ss(line);
    std::string cmd;
    int pid = 0;
    ss >> cmd >> pid;

    int sig = 0;
    if      (cmd == "KILL")    sig = SIG_AGENT_DELETE;
    else if (cmd == "SUSPEND") sig = SIG_AGENT_SUSPEND;
    else if (cmd == "WAKE")    sig = SIG_AGENT_WAKE;

    if (sig && pid > 0) {
        union sigval sv; sv.sival_int = 0;
        ::sigqueue(pid, sig, sv);
        ::write(client_fd, "OK\n", 3);
    } else {
        ::write(client_fd, "ERROR\n", 6);
    }
    ::close(client_fd);
}

static void run_control_server(int port, std::atomic<bool>& running)
{
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[control] impossible de créer le socket\n";
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
        std::cerr << "[control] bind échoué sur port " << port << "\n";
        ::close(server_fd);
        return;
    }
    ::listen(server_fd, 16);
    std::cout << "[control] serveur prêt sur port " << port << "\n";

    while (running) {
        int client_fd = ::accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;
        std::thread([client_fd]() { handle_control(client_fd); }).detach();
    }
    ::close(server_fd);
}

// ── Connexion TCP simple vers l'AMS master ────────────────────────────────────

static std::string tcp_request(const std::string& ip, int port,
                                const std::string& cmd)
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return "";

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<uint16_t>(port));
    ::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    struct timeval tv { 3, 0 };
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                  sizeof(addr)) < 0) {
        ::close(fd);
        return "";
    }
    std::string msg = cmd + "\n";
    ::write(fd, msg.c_str(), msg.size());

    std::string resp = readline_fd(fd);
    ::close(fd);
    return resp;
}

// ── Chargement de la config libconfig (pour les anciens paramètres réseau) ────

static void load_config(const std::string& path, Args& args)
{
    libconfig::Config cfg;
    try {
        cfg.readFile(path.c_str());
        if (cfg.exists("plt_address")) args.setIpAdrPlt(cfg.lookup("plt_address").c_str());
        if (cfg.exists("plt_port"))    args.setPortPlt (cfg.lookup("plt_port").c_str());
        if (cfg.exists("mng_address")) args.setIpAdrMng(cfg.lookup("mng_address").c_str());
        if (cfg.exists("mng_port"))    args.setPortMng (cfg.lookup("mng_port").c_str());
        if (cfg.exists("mon_address")) args.setIpAdrMon(cfg.lookup("mon_address").c_str());
        if (cfg.exists("mon_port"))    args.setPortMon (cfg.lookup("mon_port").c_str());
    } catch (...) {}
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    Args args;
    load_config("config.cfg", args);

    int mode = Args::getAppName(argc, argv);

    switch (mode) {

    /* ---------------------------------------------------------------- */
    case Args::AGENT_PLATFORM: {
        if (Args::argsUsageAgentPlatform(argc, argv, &args) != 0) return 1;

        // Bloquer SIGINT/SIGTERM avant de créer les threads
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigaddset(&mask, SIGTERM);
        sigprocmask(SIG_BLOCK, &mask, nullptr);

        gagent::platform::AMS            ams;
        gagent::platform::DF             df;
        gagent::platform::SlaveRegistry  registry;  // utilisé en mode master
        std::string                      effective_slave_ip; // utilisé au nettoyage slave

        // Toujours démarrer les sockets Unix locaux
        std::thread([&ams]() {
            ams.serve(gagent::platform::PlatformConfig::ams_socket_path());
        }).detach();

        std::thread([&df]() {
            df.serve(gagent::platform::PlatformConfig::df_socket_path());
        }).detach();

        // ── Mode MASTER ──────────────────────────────────────────────
        if (args.isMaster()) {
            std::string local_ip = args.localIP().empty()
                                 ? detect_local_ip()
                                 : args.localIP();
            int ams_port = std::stoi(args.getPortPlt());
            int df_port  = ams_port + 1;

            ams.setSlaveRegistry(&registry);

            std::thread([&ams, ams_port]() {
                ams.serve_tcp(ams_port);
            }).detach();

            std::thread([&df, df_port]() {
                df.serve_tcp(df_port);
            }).detach();

            registry.startWatchdog(ams, df);

            // Écrire la config : slave_ip = local_ip (les agents locaux
            // sur le master peuvent aussi utiliser TCP)
            write_config(/*master_ip=*/"", ams_port,
                         /*slave_ip=*/local_ip,
                         args.controlPort(), args.basePort());

            std::cout << "[Platform] master démarré — IP=" << local_ip
                      << " AMS TCP:" << ams_port
                      << " DF TCP:" << df_port << "\n";
        }

        // ── Mode SLAVE ──────────────────────────────────────────────
        else if (args.isSlave()) {
            // Parser l'adresse master "ip:port"
            std::string slave_str = args.slaveAddr();
            auto colon = slave_str.rfind(':');
            if (colon == std::string::npos) {
                std::cerr << "[slave] format invalide, attendu ip:port\n";
                return 1;
            }
            std::string master_ip   = slave_str.substr(0, colon);
            int         master_port = std::stoi(slave_str.substr(colon + 1));

            // Détecter/choisir l'IP de cette machine
            std::string slave_ip = args.localIP().empty()
                                 ? detect_local_ip()
                                 : args.localIP();

            // Enregistrer auprès du master (REGISTER_SLAVE)
            std::string resp = tcp_request(master_ip, master_port,
                "REGISTER_SLAVE " + std::to_string(args.controlPort()));
            if (resp.empty() || resp.substr(0, 2) != "OK") {
                std::cerr << "[slave] impossible de joindre le master "
                          << master_ip << ":" << master_port << "\n";
                return 1;
            }
            // Le master peut corriger l'IP (via getpeername) si pas de --ip
            if (args.localIP().empty() && resp.size() > 3) {
                std::string detected = resp.substr(3);
                // Trim whitespace
                while (!detected.empty() && (detected.back() == '\n'
                        || detected.back() == '\r' || detected.back() == ' '))
                    detected.pop_back();
                if (!detected.empty()) slave_ip = detected;
            }

            effective_slave_ip = slave_ip;
            std::cout << "[Platform] esclave démarré — IP=" << slave_ip
                      << " master=" << master_ip << ":" << master_port << "\n";

            // Écrire le fichier de configuration pour les agents
            write_config(master_ip, master_port, slave_ip,
                         args.controlPort(), args.basePort());

            // Thread de heartbeat
            std::thread([master_ip, master_port, slave_ip]() {
                while (true) {
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    tcp_request(master_ip, master_port,
                                "HEARTBEAT " + slave_ip);
                }
            }).detach();

            // Serveur de contrôle (KILL / SUSPEND / WAKE)
            int ctrl_port = args.controlPort();
            std::thread([ctrl_port]() {
                std::atomic<bool> running{true};
                run_control_server(ctrl_port, running);
            }).detach();
        }

        // ── Mode LOCAL ───────────────────────────────────────────────
        else {
            std::cout << "[Platform] mode local (Unix sockets uniquement)\n";
        }

        std::thread([&ams, &df]() {
            std::string line;
            while (std::getline(std::cin, line)) {
                if (line == "dump") { ams.dump(); df.dump(); }
                if (line == "quit") { std::raise(SIGTERM); break; }
            }
        }).detach();

        std::cout << "[Platform] en attente (Ctrl+C pour arrêter)...\n";
        int sig = 0;
        sigwait(&mask, &sig);

        // Nettoyage sur esclave
        if (args.isSlave() && !effective_slave_ip.empty()) {
            auto colon = args.slaveAddr().rfind(':');
            std::string master_ip   = args.slaveAddr().substr(0, colon);
            int         master_port = std::stoi(args.slaveAddr().substr(colon + 1));
            tcp_request(master_ip, master_port,
                        "DEREGISTER_SLAVE " + effective_slave_ip);
            ::unlink("/tmp/gagent.cfg");
        }

        std::cout << "[Platform] arrêt (signal " << sig << ")\n";
        _exit(0);
    }

    /* ---------------------------------------------------------------- */
    case Args::AGENT_MANAGER: {
        gagent::platform::Manager mgr;
        return mgr.run(argc, argv);
    }

    /* ---------------------------------------------------------------- */
    case Args::AGENT_MONITOR: {
        if (Args::argsUsageAgentMonitor(argc, argv, &args) != 0) return 1;
        gagent::platform::Monitor monitor(
            args.getIpAdrMon(),
            std::stoi(args.getPortMon()));
        monitor.run();
        break;
    }

    default:
        std::cerr << "Usage : agentplatform | agentmanager | agentmonitor\n";
        return 1;
    }

    return 0;
}
