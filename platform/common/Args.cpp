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

static int parse_args(int argc, char** argv,
                      Args* args,
                      const std::string& ip_key,
                      const std::string& port_key)
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

    if (vm.count("help")) { std::cout << desc << "\n"; return 0; }

    if (vm.count("ip")) {
        if      (ip_key == "mng") args->setIpAdrMng(vm["ip"].as<std::string>());
        else if (ip_key == "plt") args->setIpAdrPlt(vm["ip"].as<std::string>());
        else if (ip_key == "mon") args->setIpAdrMon(vm["ip"].as<std::string>());
    }
    if (vm.count("port")) {
        if      (port_key == "mng") args->setPortMng(vm["port"].as<std::string>());
        else if (port_key == "plt") args->setPortPlt(vm["port"].as<std::string>());
        else if (port_key == "mon") args->setPortMon(vm["port"].as<std::string>());
    }
    return 0;
}

int Args::argsUsageAgentManager (int ac, char** av, Args* a) { return parse_args(ac, av, a, "mng", "mng"); }
int Args::argsUsageAgentPlatform(int ac, char** av, Args* a) { return parse_args(ac, av, a, "plt", "plt"); }
int Args::argsUsageAgentMonitor (int ac, char** av, Args* a) { return parse_args(ac, av, a, "mon", "mon"); }
