#include "Agent.hpp"
#include "AgentFactory.hpp"
#include <gagent/platform/AMSClient.hpp>
#include <gagent/platform/PlatformConfig.hpp>
#include <gagent/platform/DFClient.hpp>
#include <gagent/utils/Logger.hpp>
#include <gagent/utils/ErrorHandler.hpp>
#include <gagent/messaging/AclMQ.hpp>
#include <gagent/messaging/ZmqTransport.hpp>
#include <sys/wait.h>  // For waitpid()
#include <unistd.h>    // For usleep()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>

#define BUFLEN 1024  				// Max length of buffer


namespace gagent {

Agent::Agent(){
	this_agent = this;
}

Agent::~Agent() {

}

void Agent::init() {
	pid_t pid;
	ppid = getpid();

	pid = fork();
	if (pid == -1) {
		throw gagent::InitializationException(
		    std::string("fork() failed: ") + std::strerror(errno));
	}
	if (pid > 0) { // the boss

		this->ppid = getpid();
		this->chldpid = pid;

		this_agent = this;
		return;
	}
	if (pid == 0) { // the child

		this->ppid = getppid();
		this->chldpid = getpid();

		this_agent = this;
		this->debug = 1;

		this->_init(debug);
	}

}

messaging::ITransport& Agent::transport() { return *transport_; }

void Agent::setTransport(std::shared_ptr<messaging::ITransport> t) {
    transport_ = std::move(t);
}

void Agent::_init(int dbg) {

	if (!transport_)
		transport_ = std::make_shared<messaging::ZmqTransport>();

	action_to_do = Agent::AGENT_UNKNOWN;

	this->agentStatus = AGENT_INITED;
	std::thread(&Agent::listener_extern_signals_Thread, this).detach();
	std::thread(&Agent::control_Thread, this).detach();


	if (dbg > 0) {
		this->udpMonitor = std::make_unique<udp_client_server::udp_client>("127.0.0.1", 40013);
	}

	std::string pid = boost::lexical_cast<std::string>(chldpid);
	std::cout << "start Agent :" + pid << std::endl;
	this->sendMsgMonitor("Start agent PID : " + pid);

	// Enregistrement FIPA auprès de l'AMS
	{
		std::string name = agentId.getAgentName();
		if (name.empty()) name = agentId.getAgentID();
		gagent::platform::AMSClient ams;
		if (ams.registerAgent(name, chldpid, get_msg_queue_name())) {
			this->agentStatus = AGENT_ACTIVE;
			LOG_JSON("agent_start",
			    {"agent", name},
			    {"pid",   std::to_string(chldpid)},
			    {"status", "active"}
			);
		} else {
			// Mode dégradé : AMS non disponible, l'agent tourne sans enregistrement FIPA.
			// Le routage inter-agents reste fonctionnel en mode local (IPC ZMQ).
			LOG_JSON("agent_start",
			    {"agent", name},
			    {"pid",   std::to_string(chldpid)},
			    {"status", "degraded_no_ams"}
			);
		}
	}

	this->setup();

	for (unsigned int i = 0; i < behaviourList.size(); i++) {
		threads.push_back(std::thread(&Agent::exthread, this, behaviourList[i]));
	}

	for (auto& th : threads) {
		th.join();
	}

	this->takeDown();
	this->doDelete();
}

void Agent::exthread(Behaviour* beh) {

	beh->onStart();
	while (!beh->done()) {
		{
			std::unique_lock<std::mutex> lck(mtxInterThred);
			cvInterThred.wait(lck, [this]{ return runingThred; });
		}
		beh->action();
	}
	beh->onEnd();

}

void Agent::listener_extern_signals_Thread() {

	try {
		boost::asio::signal_set signals(io_serv);

		signals.add(SIG_AGENT_DELETE);
		signals.add(SIG_AGENT_ACTIVE);
		signals.add(SIG_AGENT_SUSPEND);
		signals.add(SIG_AGENT_WAKE);
		signals.add(SIG_AGENT_WAIT);
		signals.add(SIG_AGENT_TRANSIT);

		signals.async_wait(
				boost::bind(&Agent::signal_handler, this->this_agent, boost::asio::placeholders::error,
						boost::asio::placeholders::signal_number));

		io_serv.run();

		for (;;) {
			io_serv.reset();
			signals.async_wait(
					boost::bind(&Agent::signal_handler, this->this_agent, boost::asio::placeholders::error,
							boost::asio::placeholders::signal_number));
			io_serv.run();
			std::this_thread::yield();
		}
	} catch (std::exception& e) {

		std::cerr << e.what() << std::endl;
	}

}

void Agent::control_Thread() {

	while (1) {
		std::unique_lock < std::mutex > lck(mtx);

		if (action_to_do <= 3){
			cv.wait(lck);
		}
		std::cout << " control_Thread bien reçu :" + boost::lexical_cast<std::string>(action_to_do) << std::endl;

		switch (action_to_do) {
		case Agent::AGENT_ACTIVE:
			std::cout << "AGENT_ACTIVE" << std::endl;
			LOG_JSON("agent_lifecycle", {"agent", agentId.getAgentName()}, {"state", "active"});
			{
				std::unique_lock < std::mutex > lck2(mtxInterThred);
				runingThred = true;
				cvInterThred.notify_all();
			}
			break;
		case Agent::AGENT_SUSPENDED:
			std::cout << "AGENT_SUSPENDED" << std::endl;
			LOG_JSON("agent_lifecycle", {"agent", agentId.getAgentName()}, {"state", "suspended"});
			{
				std::unique_lock < std::mutex > lck2(mtxInterThred);
				runingThred = false;
				cvInterThred.notify_all();
			}
			break;
		case Agent::AGENT_TRANSIT:
			std::cout << "AGENT_TRANSIT" << std::endl;
			LOG_JSON("agent_lifecycle", {"agent", agentId.getAgentName()}, {"state", "transit"});
			{
				// Sérialiser les attributs
				std::string attrs_str;
				for (auto& [k, v] : attributs)
					attrs_str += k + ":" + v + ";";
				if (attrs_str.empty()) attrs_str = "-";

				std::string type_name = agentTypeName();
				std::string ag_name   = agentId.getAgentName();
				if (ag_name.empty()) ag_name = agentId.getAgentID();

				if (type_name.empty()) {
					std::cerr << "[Agent] doMove: agentTypeName() non défini"
					             " — migration annulée\n";
					// Réactivation
					std::unique_lock<std::mutex> lk2(mtxInterThred);
					runingThred = true;
					cvInterThred.notify_all();
					break;
				}

				// Résoudre le port de migration
				auto& cfg = gagent::platform::PlatformConfig::instance();
				int mport = migration_target_.port > 0
				                ? migration_target_.port
				                : cfg.migrationPort();

				// Connexion TCP vers le serveur de migration de la cible
				int fd = ::socket(AF_INET, SOCK_STREAM, 0);
				bool migrated = false;
				if (fd >= 0) {
					struct sockaddr_in addr{};
					addr.sin_family = AF_INET;
					addr.sin_port   = htons(static_cast<uint16_t>(mport));
					::inet_pton(AF_INET,
					            migration_target_.ip.c_str(), &addr.sin_addr);
					struct timeval tv { cfg.socketTimeout(), 0 };
					::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
					::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

					if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
					              sizeof(addr)) == 0)
					{
						std::string msg = "ARRIVE " + type_name
						                + " " + ag_name
						                + " " + attrs_str + "\n";
						::write(fd, msg.c_str(), msg.size());

						char resp[16] = {};
						::read(fd, resp, sizeof(resp) - 1);
						migrated = (std::string(resp).substr(0, 2) == "OK");
					}
					::close(fd);
				}

				if (!migrated) {
					std::cerr << "[Agent] doMove: échec connexion à "
					          << migration_target_.ip << ":" << mport
					          << " — migration annulée\n";
					std::unique_lock<std::mutex> lk2(mtxInterThred);
					runingThred = true;
					cvInterThred.notify_all();
					break;
				}

				// Migration réussie : nettoyage et sortie
				std::string name = ag_name;
				LOG_JSON("agent_migrate",
				    {"agent", name},
				    {"to",    migration_target_.ip},
				    {"type",  type_name}
				);
				gagent::platform::AMSClient ams;
				ams.deregisterAgent(name);
				gagent::platform::DFClient df;
				df.deregisterAgent(name);
				gagent::messaging::acl_flush();
				_exit(0);
			}
			break;
		case Agent::AGENT_WAITING:
			std::cout << "AGENT_WAITING" << std::endl;
			LOG_JSON("agent_lifecycle", {"agent", agentId.getAgentName()}, {"state", "waiting"});
			{
				std::unique_lock < std::mutex > lck2(mtxInterThred);
				runingThred = false;
				cvInterThred.notify_all();
			}
			break;
		case Agent::AGENT_DELETED:
			std::cout << "AGENT_DELETED" << std::endl;

			break;
		case Agent::AGENT_WAKING:
			std::cout << "AGENT_WAKING" << std::endl;
			LOG_JSON("agent_lifecycle", {"agent", agentId.getAgentName()}, {"state", "waking"});
			{
				std::unique_lock < std::mutex > lck2(mtxInterThred);
				runingThred = true;
				cvInterThred.notify_all();
			}
			break;
		default:
			break;
		}
		action_to_do = 0;

	}
}



std::string Agent::get_msg_queue_name() {
	std::string aid = this->agentId.getAgentID();
	if (aid.length() > 8) {
		aid = aid.substr(0, 8);
	}
	return "/" + aid;
}


void Agent::addBehaviour(Behaviour* b) {
	this->behaviourList.push_back(b);
}

void Agent::takeDown() {

}

int Agent::sendMsgMonitor(std::string msg) {
	if (!this->udpMonitor) return 0;
	msg = agentId.getAgentID() + " -> " + msg;
	this->udpMonitor->send(msg.c_str(), BUFLEN);
	return 0;
}

AgentID Agent::getAgentId() const {
	return agentId;
}

int Agent::doDelete() {
	return doAction(Agent::AGENT_DELETED);
}

int Agent::doActivate() {
	return doAction(Agent::AGENT_ACTIVE);
}

int Agent::doSuspend() {
	return doAction(Agent::AGENT_SUSPENDED);
}

int Agent::doWait() {
	return doAction(Agent::AGENT_WAITING);
}

int Agent::doWake() {
	return doAction(Agent::AGENT_WAKING);
}

int Agent::doMove() {
	return doAction(Agent::AGENT_TRANSIT);
}

int Agent::doMove(const MigrationTarget& target) {
	migration_target_ = target;
	return doAction(Agent::AGENT_TRANSIT);
}

void Agent::setAgentName(const std::string& name) {
	agentId.setName(name);
}

int Agent::doAction(const int act) {

	bool isLocal;
	std::string pid;
	pid_t callerPid = getpid();

	union sigval sval;
	sval.sival_int = 0;

	if (this_agent->chldpid == callerPid) {
		isLocal = true;
		//std::cout << "islocal true" << std::endl;

	} else {
		isLocal = false;
		//std::cout << "islocal false  ***" << std::endl;
		//int mypid = getpid();
		//std::cout << "mypid : " << mypid << std::endl;
		//std::cout << "send signal to  : " << this_agent->chldpid << std::endl;
	}

	switch (act) {
	case Agent::AGENT_ACTIVE:
		if (isLocal) {
			std::unique_lock < std::mutex > lck(mtx);
			action_to_do = Agent::AGENT_ACTIVE;
			cv.notify_all();
		} else {
			sigqueue(chldpid, SIG_AGENT_ACTIVE, sval);
		}
		break;
	case Agent::AGENT_SUSPENDED:
		if (isLocal) {
			std::unique_lock < std::mutex > lck(mtx);
			action_to_do = Agent::AGENT_SUSPENDED;
			cv.notify_all();
		} else {
			sigqueue(chldpid, SIG_AGENT_SUSPEND, sval);
		}
		break;
	case Agent::AGENT_TRANSIT:
		if (isLocal) {
			std::unique_lock < std::mutex > lck(mtx);
			action_to_do = Agent::AGENT_TRANSIT;
			cv.notify_all();
		} else {
			sigqueue(chldpid, SIG_AGENT_TRANSIT, sval);
		}
		break;
	case Agent::AGENT_WAITING:
		if (isLocal) {
			std::unique_lock < std::mutex > lck(mtx);
			action_to_do = Agent::AGENT_WAITING;
			cv.notify_all();
		} else {

			sigqueue(chldpid, SIG_AGENT_WAIT, sval);
		}
		break;
	case Agent::AGENT_DELETED:
		if (isLocal) {
			// Le child a fini ses behaviours — nettoyage et sortie propre.
			// NE PAS s'envoyer SIGTERM à soi-même : ça déclencherait
			// cache_signal_handler qui enverrait SIGTERM à tout le groupe
			// (boucle infinie de signaux).
			pid = boost::lexical_cast<std::string>(chldpid);
			this->sendMsgMonitor("Stop agent PID : " + pid);

			// Désenregistrement FIPA : AMS + DF
			{
				std::string name = agentId.getAgentName();
				if (name.empty()) name = agentId.getAgentID();
				LOG_JSON("agent_stop",
				    {"agent", name},
				    {"pid",   std::to_string(chldpid)}
				);
				gagent::platform::AMSClient ams;
				ams.deregisterAgent(name);
				gagent::platform::DFClient df;
				df.deregisterAgent(name);
			}

			// Nettoie la queue de contrôle interne
			std::string mq_name = this->get_msg_queue_name();
			mqd_t mq = mq_open(mq_name.c_str(), O_RDONLY | O_NONBLOCK);
			if (mq != (mqd_t)-1) {
				mq_close(mq);
				mq_unlink(mq_name.c_str());
			}

			// Flush les sockets PUSH avant _exit pour que le linger ZMQ
			// puisse livrer les derniers messages (sinon _exit ferme les fd
			// sans attendre le linger).
			gagent::messaging::acl_flush();
			_exit(0);  // sortie propre du processus child
		} else {
			// Demande externe : envoie le signal au child
			sigqueue(chldpid, SIG_AGENT_DELETE, sval);
		}
		break;
	case Agent::AGENT_WAKING:
		if (isLocal) {
			std::unique_lock < std::mutex > lck(mtx);
			action_to_do = Agent::AGENT_WAKING;
			cv.notify_all();
		} else {
			sigqueue(chldpid, SIG_AGENT_WAKE, sval);
		}

		break;
	default:
		break;
	}
	return action_to_do;
}

void Agent::signal_handler(const boost::system::error_code& error, int signal_number) {
	std::cout << "received and processed signal " << std::endl;
	switch (signal_number) {
	case SIG_AGENT_DELETE:
		this_agent->doAction(Agent::AGENT_DELETED);
		break;
	case SIG_AGENT_ACTIVE:
		this_agent->doAction(Agent::AGENT_ACTIVE);
		break;
	case SIG_AGENT_SUSPEND:
		this_agent->doAction(Agent::AGENT_SUSPENDED);
		break;
	case SIG_AGENT_WAKE:
		this_agent->doAction(Agent::AGENT_WAKING);
		break;
	case SIG_AGENT_WAIT:
		this_agent->doAction(Agent::AGENT_WAITING);
		break;
	case SIG_AGENT_TRANSIT:
		this_agent->doAction(Agent::AGENT_TRANSIT);
		break;
	}
}


void Agent::attributUpdated() {
	std::map<std::string,std::string>::iterator it;
	it = this->attributs.begin();
	std::string msg("");
	while(it != this->attributs.end()){
		msg += it->first + ":" + it->second + ";" ;
		++it;
	}

	const std::string mq_name = "/envqueuemsg";
	mqd_t mq;

	std::vector<char> buffer(msg.begin(), msg.end());

	const int taille = 1000;
	const int max_msg = 5;
	struct mq_attr attr;
	attr.mq_flags   = 0;
	attr.mq_maxmsg  = max_msg;
	attr.mq_msgsize = taille;
	attr.mq_curmsgs = 0;

	mq = mq_open(mq_name.c_str(), O_WRONLY, 0666, &attr);
	if (mq == (mqd_t)-1) {
		perror("mq_open  send attributUpdated ");
		std::cout << "Error create Message Queue " << std::endl;
		return;
	}

	mq_send(mq, buffer.data(), msg.size(), 0);
	mq_close(mq);
}

void Agent::addAttribut(std::string attr) {
	this->attributs.insert(std::make_pair(attr,""));
}

void Agent::removeAttribut(std::string attr) {
	this->attributs.erase(attr);
}

void Agent::setAttribut(std::string attr, std::string val) {
	std::map<std::string,std::string>::iterator it;
	it=this->attributs.find(attr);
	if (it != this->attributs.end()) {
		this->attributs[attr] = val;
	//	this->attributUpdated();
	}
}

std::string Agent::getAttribut(std::string attr) {

	return "";
}

}
