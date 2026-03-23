/*
 * main.cpp — Point d'entrée du daemon gAgent Platform
 *
 * Le binaire détecte son rôle via son nom (ou via un symlink) :
 *
 *   agentplatform  →  AMS + DF  (registre des agents et services)
 *   agentmanager   →  gestionnaire de plateforme
 *   agentmonitor   →  affichage des logs UDP des agents
 *
 * Utilisation :
 *   ./agentplatform [--ip 127.0.0.1] [--port 40011]
 *   ./agentmonitor  [--ip 127.0.0.1] [--port 40013]
 */

#include "common/Args.hpp"
#include "ams/AMS.hpp"
#include "df/DF.hpp"
#include "monitor/Monitor.hpp"
#include "manager/Manager.hpp"

#include <gagent/platform/PlatformConfig.hpp>

#include <libconfig.h++>
#include <iostream>
#include <thread>
#include <csignal>
#include <mutex>
#include <condition_variable>

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
    } catch (const libconfig::FileIOException&) {
        std::cerr << "[platform] config.cfg introuvable, valeurs par défaut utilisées\n";
    } catch (const libconfig::ParseException& e) {
        std::cerr << "[platform] erreur config : " << e.getError() << "\n";
    }
}

int main(int argc, char* argv[])
{
    Args args;
    load_config("config.cfg", args);

    int mode = Args::getAppName(argc, argv);

    switch (mode) {

    case Args::AGENT_PLATFORM: {
        if (Args::argsUsageAgentPlatform(argc, argv, &args) != 0) return 1;

        // Bloquer SIGINT/SIGTERM avant de créer les threads
        // (les threads héritent du masque → sigwait dans main les reçoit en exclusivité)
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigaddset(&mask, SIGTERM);
        sigprocmask(SIG_BLOCK, &mask, nullptr);

        std::cout << "[Platform] AMS + DF démarrés\n";

        gagent::platform::AMS ams;
        gagent::platform::DF  df;

        std::thread ams_thread([&ams]() {
            ams.serve(gagent::platform::ams_socket_path());
        });
        ams_thread.detach();

        std::thread df_thread([&df]() {
            df.serve(gagent::platform::df_socket_path());
        });
        df_thread.detach();

        std::thread cmd_thread([&ams, &df]() {
            std::string line;
            while (std::getline(std::cin, line)) {
                if (line == "dump") { ams.dump(); df.dump(); }
                if (line == "quit") { std::raise(SIGTERM); break; }
            }
        });
        cmd_thread.detach();

        std::cout << "[Platform] en attente (Ctrl+C pour arrêter)...\n";
        int sig = 0;
        sigwait(&mask, &sig);
        std::cout << "[Platform] arrêt (signal " << sig << ")\n";
        _exit(0);
    }

    case Args::AGENT_MANAGER: {
        gagent::platform::Manager mgr;
        return mgr.run(argc, argv);
    }

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
