#include "Args.hpp"

Args::Args() {

	this->ipAdrPlt = "127.0.0.1";
	this->portPlt = "40011";

	this->ipAdrMng = "127.0.0.1";
	this->portMng = "40012";

	this->ipAdrMon = "127.0.0.1";
	this->portMon = "40013";

}

int Args::getAppName(int ac, char** av) {
	boost::filesystem::path p = av[0];
	std::string pname = p.stem().string();

	if (boost::iequals(pname, "agentmanager")) {
		return Args::AGENT_MANAGER;
	} else if (boost::iequals(pname, "agentplatform")) {
		return Args::AGENT_PLATFORM;
	} else if (boost::iequals(pname, "agentmonitor")) {
		return Args::AGENT_MONITOR;
	} else {
		std::cout << "Unknow application " << std::endl;
		return -1;
	}
	return 0;

}

int Args::argsUsageAgentManager(int ac, char** av, Args* args) {
	using namespace std;
	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()("help", "produce help message")
					  ("port",	po::value<string>(), "port")
					  ("ip",    po::value<string>(),"Ip adress");

	po::variables_map vm;

	try {
		po::store(po::parse_command_line(ac, av, desc), vm);
		po::notify(vm);
	} catch (boost::program_options::unknown_option & e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	if (vm.count("help")) {
		cout << desc << "\n";
		return 0;
	}

	if (vm.count("port")) {
		args->setPortMng(vm["port"].as<string>());
	}

	if (vm.count("ip")) {
		args->setPortMng(vm["ip"].as<string>());
	}

	return 0;
}

int Args::argsUsageAgentPlatform(int ac, char** av, Args* args) {
	using namespace std;
	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()("help", "produce help message")
					  ("port",	po::value<string>(), "port")
					  ("ip",    po::value<string>(),"Ip adress");

	po::variables_map vm;

	try {
		po::store(po::parse_command_line(ac, av, desc), vm);
		po::notify(vm);
	} catch (boost::program_options::unknown_option & e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	if (vm.count("help")) {
		cout << desc << "\n";
		return 0;
	}

	if (vm.count("port")) {
		args->setPortPlt(vm["port"].as<string>());
	}

	if (vm.count("ip")) {
		args->setPortPlt(vm["ip"].as<string>());
	}

	return 0;
}


int Args::argsUsageAgentMonitor(int ac, char** av, Args* args) {
	using namespace std;
	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()("help", "produce help message")
					  ("port",	po::value<string>(), "port")
					  ("ip",    po::value<string>(),"Ip adress");

	po::variables_map vm;

	try {
		po::store(po::parse_command_line(ac, av, desc), vm);
		po::notify(vm);
	} catch (boost::program_options::unknown_option & e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	if (vm.count("help")) {
		cout << desc << "\n";
		return 0;
	}

	if (vm.count("port")) {
		args->setPortMon(vm["port"].as<string>());
	}

	if (vm.count("ip")) {
		args->setPortMon(vm["ip"].as<string>());
	}

	return 0;
}

