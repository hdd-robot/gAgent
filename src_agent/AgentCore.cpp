/*
 * AgentCore.cpp
 *
 *  Created on: 7 août 2014
 *      Author: cyberjunky
 */

#include "AgentCore.hpp"

namespace gagent {

	std::string AgentCore::ipAdrMon = "";
	std::string AgentCore::portMon	= "";

	std::string AgentCore::ipAdrPlt = "";
	std::string AgentCore::portPlt  = "";

	std::string AgentCore::ipAdrMng = "";
	std::string AgentCore::portMng  = "";

/**
 * TODO : Intercepter SIGTERM, SIGINT (Ctrl+C) et arreter tous les processus fis du meme groupe du père
 */

void AgentCore::initAgentSystem() {
	signal(SIGINT, AgentCore::cache_signal_handler);

	libconfig::Config cfg;

	try {
		cfg.readFile("config.cfg");
	} catch (const libconfig::FileIOException &fioex) {
		std::cerr << "I/O error while reading file configuration." << std::endl;
	} catch (const libconfig::ParseException &pex) {
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
	}

	try {
		AgentCore::ipAdrPlt	= *cfg.lookup("plt_address");
		AgentCore::portPlt	= *cfg.lookup("plt_port");
		AgentCore::ipAdrMng	= *cfg.lookup("mng_address");
		AgentCore::portMng	= *cfg.lookup("mng_port");
		AgentCore::ipAdrMon	= *cfg.lookup("mon_address");
		AgentCore::portMon	= *cfg.lookup("mon_port");

	} catch (const libconfig::SettingNotFoundException &nfex) {
		std::cerr << "No setting in configuration file. The defaults values are set" << std::endl;
	} catch (...) {
		std::cerr << "Unknown exception caught\n";
		std::cerr << "the defaults values are set" << std::endl;
	}

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


void AgentCore::initEnvironnementSystem(gagent::Environnement& env, bool gui, unsigned int timer_val) {

		pid_t pid;
		pid = fork();
		if (pid == -1) {
			std::cerr << "error fork" << std::endl;
		}
		if (pid > 0) { // the boss
			return;
		}
		if (pid == 0) { // the child
			env.start(gui,timer_val);
		}
}

} /* namespace gagent */
