#include <iostream>
#include <stdio.h>
#include <string>

#include "../src_agent/AgentCore.hpp"
#include "../src_agent/Environnement.hpp"
#include "../src_agent/Agent.hpp"
#include "../src_agent/Behaviour.hpp"

using namespace gagent;

class myEnv : public gagent::Environnement {

public:
	virtual void init_env(){
		std::cout << " ------ Init Environnement ------ " << std::endl;
	};

	virtual void link_attribut(){
		this->link_id   ("id");
		this->link_name ("name");
		this->link_pos_x("x");
		this->link_pos_y("y");
		this->link_color("color");
		this->link_size ("size");
		this->link_size_x("size_x");
		this->link_size_y("size_y");
		this->link_size_z("size_z");
		this->link_shape("shape");
	};

	virtual void event_loop(){

		//std::cout<< "refresh -- " << std::endl;
	};
};


class myTickerBehaviour: public TickerBehaviour {
public:
	myTickerBehaviour(Agent* ag) : TickerBehaviour(ag,1000) { }

	void onTick(){
		//std::cout << " tictac " << std::endl << std::flush;

		this_agent->setAttribut("x",std::to_string(i++));
		this_agent->setAttribut("y",std::to_string(i++));
		this_agent->attributUpdated();
	}
private:
	int i =120 ;
};



class myAgent: public Agent {
public:
	myAgent() :
			Agent() {
	};

	void setup() {
		this_agent->addAttribut("id");
		this_agent->addAttribut("x");
		this_agent->addAttribut("y");
		this_agent->addAttribut("name");
		this_agent->addAttribut("color");
		this_agent->addAttribut("shape");
		this_agent->addAttribut("pattern");
		this_agent->addAttribut("size_x");
		this_agent->addAttribut("size_y");
	    this_agent->addAttribut("size_z");


		this_agent->setAttribut("id","agent3");
		this_agent->setAttribut("name","agent3");
		this_agent->setAttribut("color","yellow");
		this_agent->setAttribut("shape","circle");
		this_agent->setAttribut("pattern","diag3");
		this_agent->setAttribut("size_x","20");
		this_agent->setAttribut("size_y","30");
		this_agent->setAttribut("size_z","10");


		this_agent->attributUpdated();

		myTickerBehaviour* b4 = new myTickerBehaviour(this);
		addBehaviour(b4);
	}
};

int main() {
	myEnv e;
	AgentCore::initAgentSystem();
	AgentCore::initEnvironnementSystem(e, true, 2000);

	myAgent* g = new myAgent();
	g->init();

	AgentCore::syncAgentSystem();
	AgentCore::stopAgentSystem();

	return 0;
}
