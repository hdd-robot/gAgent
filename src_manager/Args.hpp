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
  string getIpAdress();
  string getPort();
  void setIp(string);
  void setPort(string);

  static int getAppName(int ,char** );
  static int argsUsageAgentManager(int , char**,Args*);
  static int argsUsageAgentPlatform(int , char**, Args*);


  static const int AGENT_MANAGER  = 10;
  static const int AGENT_PLATFORM = 20;
  static const int AGENT_MONITOR  = 30;

private: 
  string ipAdr;
  string port;
};
#endif /* ARGS_H_ */
