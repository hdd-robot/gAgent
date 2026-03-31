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


// ── CompositeBehaviour ────────────────────────────────────────────────────────

CompositeBehaviour::CompositeBehaviour() : Behaviour() {}

CompositeBehaviour::CompositeBehaviour(Agent* ag) : Behaviour(ag) {}

CompositeBehaviour::~CompositeBehaviour() {
	for (auto* b : children_) delete b;
}

void CompositeBehaviour::addSubBehaviour(Behaviour* b) {
	b->this_agent = this_agent;
	children_.push_back(b);
}

void CompositeBehaviour::removeSubBehaviour(Behaviour* b) {
	auto it = std::find(children_.begin(), children_.end(), b);
	if (it != children_.end()) children_.erase(it);
}

// ── SequentialBehaviour ───────────────────────────────────────────────────────

SequentialBehaviour::SequentialBehaviour() : CompositeBehaviour() {}

SequentialBehaviour::SequentialBehaviour(Agent* ag) : CompositeBehaviour(ag) {}

SequentialBehaviour::~SequentialBehaviour() {}

void SequentialBehaviour::onStart() {
	current_ = 0;
	if (!children_.empty())
		children_[current_]->onStart();
}

void SequentialBehaviour::action() {
	if (current_ >= children_.size()) return;

	Behaviour* cur = children_[current_];
	cur->action();

	if (cur->done()) {
		cur->onEnd();
		++current_;
		if (current_ < children_.size())
			children_[current_]->onStart();
	}
}

bool SequentialBehaviour::done() {
	return current_ >= children_.size();
}

void SequentialBehaviour::onEnd() {}

// ── ParallelBehaviour ─────────────────────────────────────────────────────────

ParallelBehaviour::ParallelBehaviour(WhenDone when)
	: CompositeBehaviour(), when_(when) {}

ParallelBehaviour::ParallelBehaviour(Agent* ag, WhenDone when)
	: CompositeBehaviour(ag), when_(when) {}

ParallelBehaviour::~ParallelBehaviour() {}

void ParallelBehaviour::onStart() {
	for (auto* b : children_) b->onStart();
}

void ParallelBehaviour::action() {
	for (auto* b : children_) {
		if (!b->done()) {
			b->action();
			if (b->done())
				b->onEnd();
		}
	}
}

bool ParallelBehaviour::done() {
	if (when_ == WhenDone::ANY) {
		for (auto* b : children_)
			if (b->done()) return true;
		return children_.empty();
	}
	// WhenDone::ALL
	for (auto* b : children_)
		if (!b->done()) return false;
	return true;
}

void ParallelBehaviour::onEnd() {}

// ── FSMBehaviour ──────────────────────────────────────────────────────────────

FSMBehaviour::FSMBehaviour() : CompositeBehaviour() {}

FSMBehaviour::FSMBehaviour(Agent* ag) : CompositeBehaviour(ag) {}

FSMBehaviour::~FSMBehaviour() {
	for (auto& [name, entry] : states_) delete entry.beh;
	// children_ n'est pas utilisé par FSMBehaviour — éviter double-delete
	children_.clear();
}

void FSMBehaviour::registerFirstState(Behaviour* b, const std::string& name) {
	b->this_agent = this_agent;
	states_[name] = { b, false };
	first_state_  = name;
}

void FSMBehaviour::registerState(Behaviour* b, const std::string& name) {
	b->this_agent = this_agent;
	states_[name] = { b, false };
}

void FSMBehaviour::registerLastState(Behaviour* b, const std::string& name) {
	b->this_agent = this_agent;
	states_[name] = { b, true };
}

void FSMBehaviour::registerTransition(const std::string& from,
                                      const std::string& to,
                                      int exit_value) {
	transitions_[from][exit_value] = to;
}

void FSMBehaviour::registerDefaultTransition(const std::string& from,
                                             const std::string& to) {
	default_transitions_[from] = to;
}

void FSMBehaviour::onStart() {
	finished_      = false;
	current_state_ = first_state_;
	if (!current_state_.empty() && states_.count(current_state_))
		states_[current_state_].beh->onStart();
}

void FSMBehaviour::action() {
	if (finished_ || current_state_.empty()) return;

	auto it = states_.find(current_state_);
	if (it == states_.end()) { finished_ = true; return; }

	StateEntry& entry = it->second;
	Behaviour*  cur   = entry.beh;

	cur->action();

	if (!cur->done()) return;

	int ev = cur->exitValue();
	cur->onEnd();

	if (entry.is_last) { finished_ = true; return; }

	// Résolution de la transition
	std::string next;
	auto it_t = transitions_.find(current_state_);
	if (it_t != transitions_.end()) {
		auto it_ev = it_t->second.find(ev);
		if (it_ev != it_t->second.end())
			next = it_ev->second;
	}
	if (next.empty()) {
		auto it_d = default_transitions_.find(current_state_);
		if (it_d != default_transitions_.end())
			next = it_d->second;
	}
	if (next.empty() || !states_.count(next)) {
		finished_ = true;
		return;
	}

	current_state_ = next;
	states_[current_state_].beh->onStart();
}

bool FSMBehaviour::done() { return finished_; }

void FSMBehaviour::onEnd() {}
}

