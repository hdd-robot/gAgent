#include <iostream>
#include <stdio.h>

#include "../src_agent/Behaviour.hpp"
#include "../src_agent/Agent.hpp"
#include "../src_agent/AgentCore.hpp"

#include "../src_agent/DFAgentDescription.hpp"
#include "../src_agent/DFService.hpp"


using namespace gagent;

class myCycle: public CyclicBehaviour {
public:
	myCycle(Agent* ag) :
			CyclicBehaviour(ag) {
	}

	void action() {
		std::cout << "." << std::flush;
		sleep(1);
	}
};

class myAgent: public Agent {
public:
	myAgent() :Agent() {};

	virtual ~myAgent() {}

	void setup() {

		DFAgentDescription dfd ;
		DFService sd1, sd2;

		dfd.setName(this->getAgentId().getAgentID());
		sd1.setServiceName("service1");
		sd2.setServiceName("service2");

		dfd.addDFServices(&sd1);
		dfd.addDFServices(&sd2);

		dfd.registerService(this,&dfd);

		//myCycle* bb = new myCycle(this);
		//addBehaviour(bb);
	}

};

int main() {

	AgentCore::initAgentSystem();
	myAgent* g2 = new myAgent();
	g2->init();

	std::cout << " " << std::endl;

	sleep(5);
	g2->doWait();
	std::cout << " " << std::endl;
	sleep(5);
	g2->doActivate();
	sleep(3);
	g2->doMove();
	sleep(3);
	g2->doSuspend();
	sleep(5);
	g2->doActivate();
	sleep(3);
	g2->doWait();
	sleep(3);
	g2->doWake();
	sleep(5);
	g2->doDelete();
	delete g2;

	AgentCore::syncAgentSystem();

	AgentCore::stopAgentSystem();

	return 0;
}

