/*
 * AgentCore.hpp
 *
 *  Created on: 7 août 2014
 *      Author: cyberjunky
 */




#ifndef AGENTCORE_HPP_
#define AGENTCORE_HPP_


#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstddef>
#include <signal.h>
#include <iostream>
#include <unistd.h>
#include <libconfig.h++>
#include "Environnement.hpp"

namespace gagent {

class AgentCore {
public:

	static void initAgentSystem();

	static void stopAgentSystem();

	static void syncAgentSystem();

	static void initEnvironnementSystem(gagent::Environnement&, bool gui=false , unsigned int timer_val = 500);


private:
	static void cache_signal_handler(int);

	static std::string ipAdrMon;
	static std::string portMon;

	static std::string ipAdrPlt;
	static std::string portPlt;

	static std::string ipAdrMng;
	static std::string portMng;

};

} /* namespace gagent */

#endif /* AGENTCORE_HPP_ */
