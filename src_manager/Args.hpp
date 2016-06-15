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
namespace gagent {
class Args {
public:

	Args();

	static int getAppName(int, char**);
	static int argsUsageAgentManager (int, char**, Args*);
	static int argsUsageAgentPlatform(int, char**, Args*);
	static int argsUsageAgentMonitor (int, char**, Args*);

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

	int getPortMng() const {
		return portMng;
	}

	void setPortMng(int portMng) {
		this->portMng = portMng;
	}

	int getPortMon() const {
		return portMon;
	}

	void setPortMon(int portMon) {
		this->portMon = portMon;
	}

	int getPortPlt() const {
		return portPlt;
	}

	void setPortPlt(int portPlt) {
		this->portPlt = portPlt;
	}

	static const int AGENT_MANAGER = 10;
	static const int AGENT_PLATFORM = 20;
	static const int AGENT_MONITOR = 30;

private:
	string ipAdrMon;
	int portMon;

	string ipAdrPlt;
	int portPlt;

	string ipAdrMng;
	int portMng;

};
}
#endif /* ARGS_H_ */
