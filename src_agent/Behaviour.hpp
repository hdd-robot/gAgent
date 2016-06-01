/*
 * Behaviour.h
 *
 *  Created on: 21 juil. 2014
 *      Author: cyberjunky
 */

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <boost/lexical_cast.hpp>
#include "udp_client_server.hpp"



#ifndef BEHAVIOUR_H_
#define BEHAVIOUR_H_

#include "Agent.hpp"
namespace gagent {

class Behaviour {
public:
	Behaviour();
	Behaviour(Agent *);
	virtual void action();
	virtual bool done();

	virtual int doDelete();
	virtual int doActivate();
	virtual int doSuspend();
	virtual int doWait();
	virtual int doWake();
	virtual int doMove();
	int doAction(const int);

	virtual ~Behaviour();

	Agent* this_agent;

};

class SimpleBehaviour: public Behaviour {
public:
	SimpleBehaviour();
	SimpleBehaviour(Agent*);
	virtual ~SimpleBehaviour();
	Agent* thisAgent;
};

class CompositeBehaviour: public Behaviour {
public:
	CompositeBehaviour();
	CompositeBehaviour(Agent*);
	virtual ~CompositeBehaviour();
};

class OneShotBehaviour: public SimpleBehaviour {
public:
	OneShotBehaviour();
	OneShotBehaviour(Agent*);
	virtual void action();
	virtual bool done();
	~OneShotBehaviour();
};

class CyclicBehaviour: public SimpleBehaviour {
public:
	CyclicBehaviour();
	CyclicBehaviour(Agent*);
	virtual void action();
	virtual bool done();
	~CyclicBehaviour();
};

class RandomBehaviour: public SimpleBehaviour {
public:
	RandomBehaviour();
	RandomBehaviour(Agent*);
	virtual void action();
	virtual bool done();
	~RandomBehaviour();
};

class WakerBehaviour: public SimpleBehaviour {
public:
	WakerBehaviour();
	WakerBehaviour(Agent*);
	virtual void action();
	virtual bool done();
	~WakerBehaviour();
};

class ParallelBehaviour: public CompositeBehaviour {
public:
	ParallelBehaviour();
	ParallelBehaviour(Agent*);
	virtual void action();
	virtual bool done();
	~ParallelBehaviour();
};

class SequentialBehaviour: public CompositeBehaviour {
public:
	SequentialBehaviour();
	SequentialBehaviour(Agent*);
	virtual void action();
	virtual bool done();
	~SequentialBehaviour();
};

class FSMBehaviour: CompositeBehaviour {
public:
	FSMBehaviour();
	FSMBehaviour(Agent*);
	virtual void action();
	virtual bool done();
	virtual ~FSMBehaviour();
};
}

#else
namespace gagent {
class Behaviour;
}

#endif /* BEHAVIOUR_H_ */

