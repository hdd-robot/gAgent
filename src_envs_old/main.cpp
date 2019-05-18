/*
 * main.cpp
 *
 *  Created on: 13 juin 2018
 *      Author: ros
 */


#include "../src_envs_old/Environnement.h"
#include "../src_envs_old/gAgentGui.h"


class myEnv : public gagent::Environnement {

public:
	virtual void init_env(){

		std::cout << " ------ init ------ " << std::endl;

	};

	virtual void link_attribut(){
		this->link_id("id");
		this->link_name("id");
		this->link_pos_x("x");
		this->link_pos_y("y");
	};

	virtual void event_loop(){

	};
};



int main(int argc, char* argv[]) {

	myEnv e;
	e.start(true);


	return 0;
}
