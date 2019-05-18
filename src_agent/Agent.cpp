#include "Agent.hpp"

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
	std::thread(&Agent::control_message, this).detach();


	if (dbg > 0) {
		this->udpMonitor = new udp_client_server::udp_client("127.0.0.1", 40013);
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

	beh->onStart();
	while (beh->done() == false) {
		std::unique_lock < std::mutex > lck2(mtxInterThred);
		while (!runingThred) {
			cvInterThred.wait(lck2);

		}
		lck2.unlock();
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



char* Agent::get_msg_queue_name() {
	char* agent_id_str = new char[9];
	this->agentId.getAgentID().copy(agent_id_str,8);
	char* mq_name = new char[10];
	std::strcpy(mq_name,"/");
	std::strcat(mq_name,agent_id_str);
	return mq_name;
}

void Agent::control_message() {

	char* mq_name = this->get_msg_queue_name();
	mqd_t  mq;

	int    taille = 100;
	int    max_msg = 5;
	char*  buffer = (char*)malloc(taille+10);

	struct mq_attr attr;
	attr.mq_flags   = 0;
	attr.mq_maxmsg  = max_msg;
	attr.mq_msgsize = taille;
	attr.mq_curmsgs = 0;

	//printf(" -> %s",mq_name);
	std::cout << std::endl;

	mq = mq_open(mq_name, O_RDONLY | O_CREAT , 0666 , &attr);

	if(mq == (mqd_t)-1){
		perror("mq_open ");
		std::cout << "Error create Message Queue " << std::endl;
		this->sendMsgMonitor("Error create Message Queue");
		return;
	}

	while (true) {
		//std::cout << "begin receve " << std::endl;
		int err = mq_receive(mq, buffer, taille, NULL);
		if (err < 0 ){
			perror("mq_receive ");
		}
		//std::string sbuffer(buffer);
		//std::cout << sbuffer << std::endl;
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
		//			//close message queue
		//			mqd_t mq = mq_open(this->get_msg_queue_name(), O_RDONLY | O_CREAT , 0660 , NULL);
		//			mq_close(mq);
		//			mq_unlink(this->get_msg_queue_name());
		//			std::cout << "close MQ -- " << std::endl;
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


void Agent::attributUpdated() {
	std::map<std::string,std::string>::iterator it;
	it = this->attributs.begin();
	std::string msg("");
	while(it != this->attributs.end()){
		msg += it->first + ":" + it->second + ";" ;
		++it;
	}

	char* mq_name = (char*)"/envqueuemsg";
	mqd_t  mq;

	char*  buffer = (char*)malloc(msg.size()+1);

	msg.copy(buffer, msg.size());

	int    taille = 1000;
		int    max_msg = 5;
	struct mq_attr attr;
	attr.mq_flags   = 0;
	attr.mq_maxmsg  = max_msg;
	attr.mq_msgsize = taille;
	attr.mq_curmsgs = 0;


	mq = mq_open(mq_name, O_WRONLY , 0666 , &attr);
	if(mq == (mqd_t)-1){
		perror("mq_open  send attributUpdated ");
		std::cout << "Error create Message Queue " << std::endl;
		return;
	}

	 mq_send(mq, buffer, msg.size(), 0);


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
