#include "Agent.hpp"

#define BUFLEN 1024  				// Max length of buffer
#define PORT_MONITOR  8888   		// PORT
#define ADR_MONITOR  "127.0.0.1"	// ADR MONITOR

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
		std::cerr << "error fork" << std::endl;
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

void Agent::_init(int dbg) {

	action_to_do = Agent::AGENT_UNKNOWN;

	this->agentStatus = AGENT_INITED;
	std::thread(&Agent::listener_extern_signals_Thread, this).detach();
	std::thread(&Agent::control_Thread, this).detach();

	if (dbg > 0) {
		this->udpMonitor = new udp_client_server::udp_client("127.0.0.1", 8888);
	}

	std::string pid = boost::lexical_cast<std::string>(chldpid);
	std::cout << "start Agent :" + pid << std::endl;
	this->sendMsgMonitor("Start agent PID : " + pid);

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
	std::unique_lock < std::mutex > lck2(mtxInterThred);
	while (!runingThred) {
		cvInterThred.wait(lck2);
	}
	beh->action();
	while (beh->done() == false) {
		beh->action();
	}

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

		if (action_to_do <= 3)
			cv.wait(lck);
		std::cout << " control_Thread bien reçu :" + boost::lexical_cast<std::string>(action_to_do) << std::endl;

		switch (action_to_do) {
		case Agent::AGENT_ACTIVE:
			std::cout << "AGENT_ACTIVE" << std::endl;
			{
				std::unique_lock < std::mutex > lck2(mtxInterThred);
				runingThred = true;
				cvInterThred.notify_all();
			}
			break;
		case Agent::AGENT_SUSPENDED:
			std::cout << "AGENT_SUSPENDED" << std::endl;
			{
				std::unique_lock < std::mutex > lck2(mtxInterThred);
				runingThred = false;
				cvInterThred.notify_all();
			}
			break;
		case Agent::AGENT_TRANSIT:
			std::cout << "AGENT_TRANSIT" << std::endl;
			// pas définie encore
			break;
		case Agent::AGENT_WAITING:
			std::cout << "AGENT_WAITING" << std::endl;
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

void Agent::addBehaviour(Behaviour* b) {
	this->behaviourList.push_back(b);
}

void Agent::takeDown() {

}

int Agent::sendMsgMonitor(std::string msg) {
	msg = boost::lexical_cast<std::string>(agentId.getAgentID()) + " -> " + msg;
	this->udpMonitor->send((char*) msg.c_str(), BUFLEN);
	return 0;
}

AgentID Agent::getAgentId() {
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
			pid = boost::lexical_cast<std::string>(chldpid);
			this->sendMsgMonitor("Stop agent PID : " + pid);
			kill(chldpid, SIGKILL);  // TODO : chldpid ????
		} else {
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

}
