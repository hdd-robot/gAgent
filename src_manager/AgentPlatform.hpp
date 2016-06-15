#include "Args.hpp"
#include "../src_agent/DFService.hpp"
#ifndef AGENTPLATFORM_H_
#define AGENTPLATFORM_H_

namespace gagent {
#define BUFLENAF 1024  			//Max length of buffer
class AgentPlatform {
public:
	AgentPlatform();
	void startAgentPlatform(Args*);
	void stopAgentPlaform();

	void insertAgent(string name);
	void removeAgent(string name);
	void insertService(string agentName);
	void removeService();

private:
	static std::map<string, std::list<DFService>> serviceList;

};

#endif /*AGENTPLATFORM */
}
