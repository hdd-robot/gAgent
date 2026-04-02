/*
 * Agent.hpp
 *
 *  Created on: 3 jun 2014
 *      Author: HD <hdd@ai.univ-paris8.fr>
 */

#ifndef AGENT_H_
#define AGENT_H_

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include <string>
#include <boost/lexical_cast.hpp>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <cstddef>

#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/signal_set.hpp>
#include <mqueue.h>
#include <map>


#include <memory>
#include <gagent/utils/udp_client_server.hpp>
#include <gagent/messaging/ITransport.hpp>
#include "Behaviour.hpp"
#include "AgentID.hpp"



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

	// Nom du type d'agent — à surcharger pour permettre la migration.
	// Doit correspondre au nom enregistré dans AgentFactory::registerType().
	virtual std::string agentTypeName() const { return ""; }

	int doDelete();
	int doActivate();
	int doSuspend();
	int doWait();
	int doWake();
	int doMove();

	// Migration vers un nœud distant.
	// Sérialise les attributs, crée l'agent sur la cible, puis se supprime.
	// Requiert agentTypeName() non vide et AgentFactory::startMigrationServer()
	// démarré sur la machine cible.
	struct MigrationTarget {
		std::string ip;
		int         port = -1;  // -1 = PlatformConfig::migrationPort()
	};
	int doMove(const MigrationTarget& target);

	int doAction(const int);

	// Permet de fixer le nom avant init() (utile lors de la migration).
	void setAgentName(const std::string& name);

	void addBehaviour(Behaviour*);
	void removeBehaviour(Behaviour*);

	AgentID getAgentId() const;

	// Transport de messagerie FIPA — injecté à l'init, ZmqTransport par défaut.
	// Remplacer via setTransport() avant setup() pour changer de transport.
	messaging::ITransport& transport();
	void setTransport(std::shared_ptr<messaging::ITransport> t);

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



	void addAttribut(std::string);
	void removeAttribut(std::string);

	void setAttribut(std::string, std::string);
	std::string getAttribut(std::string);

	void attributUpdated(); // todo a rendtre privé

private:

	std::shared_ptr<messaging::ITransport> transport_;
	MigrationTarget migration_target_;

	std::map<std::string,std::string> attributs;

	std::string get_msg_queue_name();

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
	std::string agentName;
	std::string agentDateCreate;
	int debug;

	std::unique_ptr<udp_client_server::udp_client> udpMonitor;

};

}
#else
namespace gagent {
	class Agent;
}
#endif /* AGENT_H_ */
