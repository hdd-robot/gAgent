#include "AgentPlatform.hpp"

/* constructor */
AgentPlatform::AgentPlatform() {

}

void AgentPlatform::startAgentPlatform(Args* args) {
	std::cout << "Start AgentPlatform " << endl;
	char msg[BUFLENAF];
	udp_client_server::udp_server server(args->getIpAdrPlt(),args->getPortPlt());
	while (1) {
		server.recv(msg, BUFLENAF);
		fprintf(stdout, " %s \n", msg);
	}

}

void AgentPlatform::stopAgentPlaform() {

}
