/*
 * Agent.hpp
 *
 *  Created on: 3 jun 2014
 *      Author: HD <hdd@ai.univ-paris8.fr>
 */

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <signal.h>
#include <vector>
#include <unistd.h>
#include <boost/lexical_cast.hpp>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <signal.h>
#include <cstddef>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/impl/io_service.hpp>

#include <boost/bind.hpp>

#include "udp_client_server.hpp"
#include "Behaviour.hpp"
#include "AgentID.hpp"

#ifndef AGENT_H_
#define AGENT_H_


#define SIG_AGENT_DELETE 	(__SIGRTMIN + 2)
#define SIG_AGENT_ACTIVE 	(__SIGRTMIN + 3)
#define SIG_AGENT_SUSPEND 	(__SIGRTMIN + 4)
#define SIG_AGENT_WAKE 		(__SIGRTMIN + 5)
#define SIG_AGENT_WAIT 		(__SIGRTMIN + 6)
#define SIG_AGENT_TRANSIT 	(__SIGRTMIN + 7)

namespace gagent {

class Agent {
public:

	Agent();

	virtual ~Agent();

	bool is_notification();

	void* scheduler(void *);
	void exthread(Behaviour*);

	void control_Thread();
	void listener_extern_signals_Thread();

	void init();
	void _init(int);

	virtual void setup() = 0; // a redéfinir obligatoirement
	virtual void takeDown();  // a redéfinir au choix

	int doDelete();
	int doActivate();
	int doSuspend();
	int doWait();
	int doWake();
	int doMove();
	int doAction(const int);

	void addBehaviour(Behaviour*);
	void removeBehaviour(Behaviour*);

	AgentID getAgentId();

	int agentStatus;
	static const int AGENT_UNKNOWN 	= 0;	// Agent not created yet
	static const int AGENT_CREATED 	= 1;	// Agnet created
	static const int AGENT_INITED 	= 2;	// Agent est lancé mais non enregistré auprès de l'AMS, aucun nom aucune adress
	static const int AGENT_ACTIVE 	= 3;	// Agent est répértorié auprès de lAMS et peut accéder aux services
	static const int AGENT_SUSPENDED= 4;	// Tous les behaviors de l'agent sont suspondus
	static const int AGENT_TRANSIT 	= 5;	// Agent en migration vers une autre station
	static const int AGENT_WAITING 	= 6;	// Tous les behaviors de l'agent sont temporairement suspondu
	static const int AGENT_DELETED 	= 7;	// L'exécution de l'agent est terminée et n'est plus répértorié au sein de l'AMS
	static const int AGENT_WAKING 	= 8; 	// reprendre le travail;


	int sendMsgMonitor(std::string);
	pid_t chldpid;
	pid_t ppid;

	Agent* this_agent;

	void signal_handler(const boost::system::error_code& error, int signal_number);

protected:

private:
	int action_to_do;
	boost::asio::io_service io_serv;

	std::mutex mtx;
	std::mutex mtxInterThred;

	std::condition_variable cv;
	std::condition_variable cvInterThred;

	bool runingThred = true;

	std::vector<Behaviour*> behaviourList;
	std::vector<std::thread> threads;
	std::thread::id control_thread_id;
	AgentID agentId;
	char* agentName;
	char* agentDateCreate;
	int debug;

	udp_client_server::udp_client * udpMonitor;

};

}
#else
namespace gagent {
	class Agent;
}
#endif /* AGENT_H_ */
