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

void Behaviour::onStart() {

}

void Behaviour::onEnd() {

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
int Behaviour::doAction(int a) {
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
WakerBehaviour::WakerBehaviour() :SimpleBehaviour() {

}

WakerBehaviour::WakerBehaviour(Agent* ag,unsigned int timer) : SimpleBehaviour(ag) {
	this->internalTimer= timer;
}

void WakerBehaviour::action() {
	this->internalTimer = this->internalTimer - 1;
	std::chrono::duration<double, std::milli> timeToWait(1.0);
	std::chrono::duration<double, std::milli> elapsed;
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	do{
		end = std::chrono::high_resolution_clock::now();
		elapsed = end-start;
	}while(elapsed <= timeToWait);

}

bool WakerBehaviour::done() {
	if(internalTimer == 0){
		this->onWake();
		return true;
	}
	return false;
}

void WakerBehaviour::onWake() {

}

WakerBehaviour::~WakerBehaviour() {
}


// TickerBehaviour

TickerBehaviour::TickerBehaviour() :SimpleBehaviour() {

}

TickerBehaviour::TickerBehaviour(Agent* ag,unsigned int timer) : SimpleBehaviour(ag) {
	this->internalTimer= timer;
}

void TickerBehaviour::action() {

	std::chrono::duration<double, std::milli> elapsed;
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> timeToWait(this->internalTimer);

	do{
		end = std::chrono::high_resolution_clock::now();
		elapsed = end-start;
	}while(elapsed <= timeToWait);

	this->onTick();
}

bool TickerBehaviour::done() {
	return false;
}

void TickerBehaviour::onTick() {

}

TickerBehaviour::~TickerBehaviour() {
}


/**
 * CompositeBehaviour
 */
/*
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
*/
}

