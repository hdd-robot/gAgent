/*
 * Args.hpp
 *
 *  Created on: 3 jun 2014
 *  Author: HD <hdd@ai.univ-paris8.fr>
 */

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

#ifndef ARGS_H_
#define ARGS_H_
using namespace std;
class Args {
public:

	Args();

	static int getAppName(int, char**);
	static int argsUsageAgentManager(int, char**, Args*);
	static int argsUsageAgentPlatform(int, char**, Args*);
	static int argsUsageAgentMonitor(int, char**, Args*);

	const string& getIpAdrMng() const {
		return ipAdrMng;
	}

	void setIpAdrMng(const string& ipAdrMng) {
		this->ipAdrMng = ipAdrMng;
	}

	const string& getIpAdrMon() const {
		return ipAdrMon;
	}

	void setIpAdrMon(const string& ipAdrMon) {
		this->ipAdrMon = ipAdrMon;
	}

	const string& getIpAdrPlt() const {
		return ipAdrPlt;
	}

	void setIpAdrPlt(const string& ipAdrPlt) {
		this->ipAdrPlt = ipAdrPlt;
	}

	const string& getPortMng() const {
		return portMng;
	}

	void setPortMng(const string& portMng) {
		this->portMng = portMng;
	}

	const string& getPortMon() const {
		return portMon;
	}

	void setPortMon(const string& portMon) {
		this->portMon = portMon;
	}

	const string& getPortPlt() const {
		return portPlt;
	}

	void setPortPlt(const string& portPlt) {
		this->portPlt = portPlt;
	}

	static const int AGENT_MANAGER = 10;
	static const int AGENT_PLATFORM = 20;
	static const int AGENT_MONITOR = 30;

private:
	string ipAdrMon;
	string portMon;

	string ipAdrPlt;
	string portPlt;

	string ipAdrMng;
	string portMng;

};
#endif /* ARGS_H_ */
