#include <iostream>
#include <iomanip>
#include <libconfig.h++>

#include "Args.hpp"
#include "AgentPlatform.hpp"
#include "AgentManager.hpp"
#include "udp_client_server.hpp"

#define BUFLEN 1024  			//Max length of buffer
#define PORT_MONITOR  8888   		//PORT
#define ADR_MONITOR  "127.0.0.1"	//ADR MONITOR

int main(int ac, char* av[]) {
	libconfig::Config cfg;
	Args* args = new Args;

	try {
		cfg.readFile("example.cfg");
	} catch (const FileIOException &fioex) {
		std::cerr << "I/O error while reading file configuration." << std::endl;
		return (EXIT_FAILURE);
	} catch (const ParseException &pex) {
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
				<< " - " << pex.getError() << std::endl;
		return (EXIT_FAILURE);
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
		udp_client_server::udp_server server("127.0.0.1", PORT_MONITOR);
		while (1) {
			num++;
			server.recv(msg, BUFLEN);
			fprintf(stdout, "%8.6d : %s \n", num, msg);

		}
		break;


	}

	return 0;

}

