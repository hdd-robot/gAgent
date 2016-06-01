/*
 * AgentCore.cpp
 *
 *  Created on: 7 août 2014
 *      Author: cyberjunky
 */

#include "AgentCore.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <cstddef>
#include <signal.h>
#include <iostream>
#include <unistd.h>

namespace gagent {

/**
 * TODO : Intercepter SIGTERM, SIGINT (Ctrl+C) et arreter tous les processus fis du meme groupe du père
 */

void AgentCore::initAgentSystem() {
	signal(SIGINT, AgentCore::cache_signal_handler);
}

void AgentCore::stopAgentSystem() {

}

void AgentCore::syncAgentSystem() {
	waitpid(-1, NULL, 0);
}

void AgentCore::cache_signal_handler(int _ignored) {
	std::cout << "killall " << std::endl;
	int pid = getpid();
	kill(-pid, SIGQUIT);
}

} /* namespace gagent */
