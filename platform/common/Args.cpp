#include "Args.hpp"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

namespace po = boost::program_options;

Args::Args() {}

int Args::getAppName(int argc, char** argv)
{
    boost::filesystem::path p = argv[0];
    std::string name = p.stem().string();

    if      (boost::iequals(name, "agentmanager"))  return AGENT_MANAGER;
    else if (boost::iequals(name, "agentplatform")) return AGENT_PLATFORM;
    else if (boost::iequals(name, "agentmonitor"))  return AGENT_MONITOR;

    std::cerr << "Application inconnue : " << name << "\n";
    return -1;
}

int Args::argsUsageAgentManager(int argc, char** argv, Args* args)
{
    (void)argc; (void)argv; (void)args;
    return 0;  // Manager n'a pas d'options réseau à ce niveau
}

int Args::argsUsageAgentPlatform(int argc, char** argv, Args* args)
{
    po::options_description desc("agentplatform — options");
    desc.add_options()
        ("help",
            "Affiche l'aide")
        ("master",
            "Démarre en mode master (AMS+DF TCP activés)")
        ("slave", po::value<std::string>(),
            "Démarre en mode esclave, ex: --slave 192.168.1.10:40011")
        ("ip",   po::value<std::string>(),
            "Adresse IP de cette machine (auto-détectée si absent)")
        ("port", po::value<std::string>(),
            "Port AMS (défaut 40011, DF = port+1)")
        ("control-port", po::value<int>(),
            "Port du serveur de contrôle sur l'esclave (défaut 40015)")
        ("base-port", po::value<int>(),
            "Port de base pour les endpoints ZMQ TCP (défaut 50000)");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::unknown_option& e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    if (vm.count("help")) { std::cout << desc << "\n"; return 1; }

    if (vm.count("master"))       args->master_       = true;
    if (vm.count("slave"))        args->slave_addr_   = vm["slave"].as<std::string>();
    if (vm.count("ip"))           args->local_ip_     = vm["ip"].as<std::string>();
    if (vm.count("port"))         args->setPortPlt(vm["port"].as<std::string>());
    if (vm.count("control-port")) args->control_port_ = vm["control-port"].as<int>();
    if (vm.count("base-port"))    args->base_port_    = vm["base-port"].as<int>();

    return 0;
}

int Args::argsUsageAgentMonitor(int argc, char** argv, Args* args)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help", "Affiche l'aide")
        ("ip",   po::value<std::string>(), "Adresse IP")
        ("port", po::value<std::string>(), "Port");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::unknown_option& e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    if (vm.count("ip"))   args->setIpAdrMon(vm["ip"].as<std::string>());
    if (vm.count("port")) args->setPortMon(vm["port"].as<std::string>());
    return 0;
}
