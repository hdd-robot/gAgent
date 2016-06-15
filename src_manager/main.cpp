#include <iostream>
#include <iomanip>
#include <libconfig.h++>

#include "Args.hpp"
#include "AgentPlatform.hpp"
#include "AgentManager.hpp"

#include "../src_agent/udp_client_server.hpp"
#include "../src_agent/DFAgentDescription.hpp"
#include "../src_agent/DFService.hpp"

using namespace gagent;
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
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - "
				<< pex.getError() << std::endl;
		return (EXIT_FAILURE);
	}

	try {
		string plt_address = cfg.lookup("plt_address");
		args->setIpAdrPlt(plt_address);
	} catch (const libconfig::SettingNotFoundException &nfex) {
		cerr << "No 'plt_address' setting in configuration file. Then default value is set" << endl;
	}catch (...) {cerr << "Error plt_address setting in configuration file. Then default value is set" << endl;}

	try {
		int plt_port = cfg.lookup("plt_port");
		args->setPortPlt(plt_port);
	} catch (const libconfig::SettingNotFoundException &nfex) {
		cerr << "No 'plt_port' setting in configuration file. Then default value is set" << endl;
	}  catch (...) {cerr << "Error plt_port setting in configuration file. Then default value is set" << endl;}

	try {
		string mng_address = cfg.lookup("mng_address");
		args->setIpAdrMng(mng_address);
	} catch (const libconfig::SettingNotFoundException &nfex) {
		cerr << "No 'mng_address' setting in configuration file. Then default value is set" << endl;
	}catch (...) {cerr << "Error mng_address setting in configuration file. Then default value is set" << endl;}

	try {
		int mng_port = cfg.lookup("mng_port");
		args->setPortMng(mng_port);
	} catch (const libconfig::SettingNotFoundException &nfex) {
		cerr << "No 'mng_port' setting in configuration file. Then default value is set" << endl;
	}  catch (...) {cerr << "Error mng_port setting in configuration file. Then default value is set" << endl;}

	try {
		string mon_address = cfg.lookup("mon_address");
		args->setIpAdrMon(mon_address);
	} catch (const libconfig::SettingNotFoundException &nfex) {
		cerr << "No 'mon_address' setting in configuration file. Then default value is set" << endl;
	}catch (...) {cerr << "Error mon_address setting in configuration file. Then default value is set" << endl;}

	try {
		int mon_port = cfg.lookup("mon_port");
		args->setPortMon(mon_port);
	} catch (const libconfig::SettingNotFoundException &nfex) {
		cerr << "No 'mon_port' setting in configuration file. Then default value is set" << endl;
	}  catch (...) {cerr << "Error mon_port setting in configuration file. Then default value is set" << endl;}


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
			udp_client_server::udp_server server(args->getIpAdrMon(), args->getPortMon());
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

