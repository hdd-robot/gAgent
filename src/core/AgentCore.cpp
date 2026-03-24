/*
 * AgentCore.cpp
 *
 *  Created on: 7 août 2014
 *      Author: cyberjunky
 */

#include "AgentCore.hpp"
#include <gagent/platform/PlatformConfig.hpp>
#include <thread>

namespace gagent {

	std::string AgentCore::ipAdrMon = "";
	std::string AgentCore::portMon	= "";

	std::string AgentCore::ipAdrPlt = "";
	std::string AgentCore::portPlt  = "";

	std::string AgentCore::ipAdrMng = "";
	std::string AgentCore::portMng  = "";

/**
 * @brief Initialize the agent system with proper signal handling
 * 
 * Sets up signal handlers for SIGINT, SIGTERM, and SIGQUIT to ensure
 * graceful shutdown of all child processes in the process group.
 */
void AgentCore::initAgentSystem() {
	// Register signal handlers for graceful shutdown
	signal(SIGINT, AgentCore::cache_signal_handler);
	signal(SIGTERM, AgentCore::cache_signal_handler);
	signal(SIGQUIT, AgentCore::cache_signal_handler);

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
	while (waitpid(-1, NULL, 0) > 0);
}

void AgentCore::cache_signal_handler(int signal_num) {
	const char* signal_name = "UNKNOWN";
	switch(signal_num) {
		case SIGINT:  signal_name = "SIGINT";  break;
		case SIGTERM: signal_name = "SIGTERM"; break;
		case SIGQUIT: signal_name = "SIGQUIT"; break;
	}
	
	std::cout << "Caught signal " << signal_name << " (" << signal_num 
	          << "), shutting down all agents..." << std::endl;

	// Get current process group ID
	pid_t pgid = getpgrp();
	
	// Send SIGTERM to all processes in the process group
	// Using negative PID sends to the entire process group
	if (kill(-pgid, SIGTERM) == -1) {
		perror("Failed to send SIGTERM to process group");
	}
	
	// Give processes time to clean up
	sleep(1);
	
	// Force kill any remaining processes
	kill(-pgid, SIGKILL);
	
	exit(EXIT_SUCCESS);
}


void AgentCore::initEnvironnementSystem(gagent::Environnement& env) {
	pid_t pid = fork();
	if (pid == -1) {
		std::cerr << "error fork" << std::endl;
	}
	if (pid > 0) return;
	if (pid == 0) {
		std::string sock_path = gagent::platform::env_socket_path();
		std::thread(&gagent::Environnement::serve, &env, sock_path).detach();
		env.start();    // bloque dans event_loop() jusqu'à la fin de la simulation
		_exit(0);       // event_loop() terminée — sortie propre sans dupliquer les agents
	}
}

} /* namespace gagent */
