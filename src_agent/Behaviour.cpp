/*
 * Behaviour.cpp
 *
 *  Created on: 21 juil. 2014
 *      Author: cyberjunky
 */

#include "Behaviour.hpp"
namespace gagent {
/**
 * Behaviour
 */

Behaviour::Behaviour() {
	this_agent = NULL;
}

Behaviour::Behaviour(Agent* ag) {
	this_agent = ag;

}

void Behaviour::action() {

}
bool Behaviour::done() {
	return true;
}

Behaviour::~Behaviour() {

}


int Behaviour::doDelete() {
	return this_agent->doDelete();
}

int Behaviour::doActivate() {
	return this_agent->doActivate();
}

int Behaviour::doSuspend() {
	return this_agent->doSuspend();
}

int Behaviour::doWait() {
	return this_agent->doWait();
}

int Behaviour::doWake() {
	return this_agent->doWake();
}

int Behaviour::doMove() {
	return this_agent->doMove();
}
int Behaviour::doAction(int a ) {
	return this_agent->doAction(a);
}

/**
 * SimpleBehaviour
 */

SimpleBehaviour::SimpleBehaviour() :
		Behaviour() {

}

SimpleBehaviour::SimpleBehaviour(Agent* ag) :
		Behaviour(ag) {

}

SimpleBehaviour::~SimpleBehaviour() {
}

// OneShotBehaviour

OneShotBehaviour::OneShotBehaviour() :
		SimpleBehaviour() {

}

OneShotBehaviour::OneShotBehaviour(Agent* ag) :
		SimpleBehaviour(ag) {

}
void OneShotBehaviour::action() {

}

bool OneShotBehaviour::done() {
	return true;
}

OneShotBehaviour::~OneShotBehaviour() {
}

//  CyclicBehaviour

CyclicBehaviour::CyclicBehaviour() :
		SimpleBehaviour() {

}

CyclicBehaviour::CyclicBehaviour(Agent* ag) :
		SimpleBehaviour(ag) {

}
void CyclicBehaviour::action() {

}

bool CyclicBehaviour::done() {
	return false;
}

CyclicBehaviour::~CyclicBehaviour() {
}

// WakerBehaviour
WakerBehaviour::WakerBehaviour() :
		SimpleBehaviour() {

}

WakerBehaviour::WakerBehaviour(Agent* ag) :
		SimpleBehaviour(ag) {

}
void WakerBehaviour::action() {

}

bool WakerBehaviour::done() {
	return false;
}
WakerBehaviour::~WakerBehaviour() {
}

//	RandomBehaviour
RandomBehaviour::RandomBehaviour() :
		SimpleBehaviour() {

}

RandomBehaviour::RandomBehaviour(Agent* ag) :
		SimpleBehaviour(ag) {

}
void RandomBehaviour::action() {

}

bool RandomBehaviour::done() {
	return false;
}
RandomBehaviour::~RandomBehaviour() {
}

/**
 * CompositeBehaviour
 */
CompositeBehaviour::CompositeBehaviour() :
		Behaviour() {

}

CompositeBehaviour::CompositeBehaviour(Agent* ag) :
		Behaviour(ag) {

}

CompositeBehaviour::~CompositeBehaviour() {
}

//  ParallelBehaviour
ParallelBehaviour::ParallelBehaviour() :
		CompositeBehaviour() {

}

ParallelBehaviour::ParallelBehaviour(Agent* ag) :
		CompositeBehaviour(ag) {

}

void ParallelBehaviour::action() {

}

bool ParallelBehaviour::done() {
	return false;
}

ParallelBehaviour::~ParallelBehaviour() {
}

//  SequentialBehaviour
SequentialBehaviour::SequentialBehaviour() :
		CompositeBehaviour() {

}

SequentialBehaviour::SequentialBehaviour(Agent* ag) :
		CompositeBehaviour(ag) {

}
void SequentialBehaviour::action() {

}

bool SequentialBehaviour::done() {
	return false;
}




SequentialBehaviour::~SequentialBehaviour() {
}

// FSMBehaviour
FSMBehaviour::FSMBehaviour() :
		CompositeBehaviour() {

}

FSMBehaviour::FSMBehaviour(Agent* ag) :
		CompositeBehaviour(ag) {

}
void FSMBehaviour::action() {

}

bool FSMBehaviour::done() {
	return false;
}

FSMBehaviour::~FSMBehaviour() {
}


}


