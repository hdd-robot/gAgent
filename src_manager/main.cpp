#include <iostream>
#include <iomanip>
#include <libconfig.h++>

#include "Args.hpp"
#include "AgentPlatform.hpp"
#include "AgentManager.hpp"
#include "udp_client_server.hpp"

#define BUFLEN 1024  			//Max length of buffer

int main(int ac, char* av[]) {
	libconfig::Config cfg;
	Args* args = new Args;

	try {
		cfg.readFile("config.cfg");

	} catch (const libconfig::FileIOException &fioex) {
		std::cerr << "I/O error while reading file configuration." << std::endl;
		return (EXIT_FAILURE);
	} catch (const libconfig::ParseException &pex) {
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
				<< " - " << pex.getError() << std::endl;
		return (EXIT_FAILURE);
	}

	try{
		args->setIpAdrPlt(cfg.lookup("plt_address"));
		args->setPortPlt (cfg.lookup("plt_port"));
		args->setIpAdrMng(cfg.lookup("mng_address"));
		args->setPortMng (cfg.lookup("mng_port"));
		args->setIpAdrMon(cfg.lookup("mon_address"));
		args->setPortMon (cfg.lookup("mon_port"));
	}
	catch(const libconfig::SettingNotFoundException &nfex){
		cerr << "No setting in configuration file. The defaults values are set" << endl;
	}
	catch (...)
	{
	    std::cerr << "Unknown exception caught\n";
	    cerr << "the defaults values are set" << endl;
	}


	int ret = Args::getAppName(ac, av);
	switch (ret) {
	case Args::AGENT_MANAGER:
		if (Args::argsUsageAgentManager(ac, av, args) == 0) {
			AgentManager* am = new AgentManager();
			am->startAgentManager(args);
		}

		break;
	case Args::AGENT_PLATFORM:
		if (Args::argsUsageAgentPlatform(ac, av, args) == 0) {
			AgentPlatform* ap = new AgentPlatform();
			ap->startAgentPlatform(args);
		}
		break;
	case Args::AGENT_MONITOR:
		char msg[BUFLEN];
		int num = 0;
		if (Args::argsUsageAgentMonitor(ac, av, args) == 0) {
			udp_client_server::udp_server server(args->getIpAdrMon(), boost::lexical_cast<int>(args->getPortMon()));
			while (1) {
				num++;
				server.recv(msg, BUFLEN);
				fprintf(stdout, "%8.6d : %s \n", num, msg);

			}
		}
		break;


}

	return 0;

}

