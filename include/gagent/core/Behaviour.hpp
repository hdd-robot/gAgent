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
#include <map>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <boost/lexical_cast.hpp>
#include <gagent/utils/udp_client_server.hpp>



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

	virtual void onStart();
	virtual void onEnd();

	// Valeur de sortie utilisée par FSMBehaviour pour déterminer la transition.
	// À surcharger dans les états FSM pour retourner une valeur significative.
	virtual int exitValue() { return 0; }

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

/*
 *  SimpleBehaviour
 */

class SimpleBehaviour: public Behaviour {
public:
	SimpleBehaviour();
	SimpleBehaviour(Agent*);
	virtual ~SimpleBehaviour();
	Agent* thisAgent;
};



class OneShotBehaviour: public SimpleBehaviour {
public:
	OneShotBehaviour();
	OneShotBehaviour(Agent*);
	virtual void action();
	virtual bool done() final;
	virtual ~OneShotBehaviour();
};

class CyclicBehaviour: public SimpleBehaviour {
public:
	CyclicBehaviour();
	CyclicBehaviour(Agent*);
	virtual void action();
	virtual bool done() final;
	virtual ~CyclicBehaviour();
};

/*
 *  WakerBehaviour
 */

class WakerBehaviour: public SimpleBehaviour {
public:
	WakerBehaviour();
	WakerBehaviour(Agent*,unsigned int);
	virtual void action() final;
	virtual bool done() final;
	virtual void onWake();
	virtual ~WakerBehaviour();
private :
	unsigned int internalTimer;
};


class TickerBehaviour: public SimpleBehaviour {
public:
	TickerBehaviour();
	TickerBehaviour(Agent*,unsigned int);
	virtual void action() final;
	virtual bool done() final;
	virtual void onTick();
	virtual ~TickerBehaviour();
private :
	unsigned int internalTimer;
};

// ── CompositeBehaviour ────────────────────────────────────────────────────────

/**
 * Behaviour de base pour les comportements composites.
 * Gère une liste de sous-behaviours et leur cycle de vie.
 */
class CompositeBehaviour : public Behaviour {
public:
	CompositeBehaviour();
	explicit CompositeBehaviour(Agent*);
	virtual ~CompositeBehaviour();

	void addSubBehaviour(Behaviour* b);
	void removeSubBehaviour(Behaviour* b);

protected:
	std::vector<Behaviour*> children_;
};

// ── SequentialBehaviour ───────────────────────────────────────────────────────

/**
 * Exécute les sous-behaviours séquentiellement, un par un dans l'ordre
 * d'ajout. Termine quand tous les enfants sont terminés.
 */
class SequentialBehaviour : public CompositeBehaviour {
public:
	SequentialBehaviour();
	explicit SequentialBehaviour(Agent*);
	virtual ~SequentialBehaviour();

	void action()  override;
	bool done()    override;
	void onStart() override;
	void onEnd()   override;

private:
	std::size_t current_ = 0;
};

// ── ParallelBehaviour ─────────────────────────────────────────────────────────

/**
 * Exécute tous les sous-behaviours à chaque tick.
 *
 * WhenDone::ALL  — termine quand tous les enfants sont terminés (défaut)
 * WhenDone::ANY  — termine dès qu'un enfant est terminé
 */
class ParallelBehaviour : public CompositeBehaviour {
public:
	enum class WhenDone { ALL, ANY };

	explicit ParallelBehaviour(WhenDone when = WhenDone::ALL);
	ParallelBehaviour(Agent*, WhenDone when = WhenDone::ALL);
	virtual ~ParallelBehaviour();

	void action()  override;
	bool done()    override;
	void onStart() override;
	void onEnd()   override;

private:
	WhenDone when_;
};

// ── FSMBehaviour ──────────────────────────────────────────────────────────────

/**
 * Machine à états finis (FSM).
 *
 * Usage :
 *   registerFirstState(new StateA(ag), "A");
 *   registerState     (new StateB(ag), "B");
 *   registerLastState (new StateC(ag), "C");
 *   registerTransition("A", "B", 0);   // StateA::exitValue() == 0 → B
 *   registerTransition("A", "C", 1);   // StateA::exitValue() == 1 → C
 *   registerDefaultTransition("B", "C");
 *
 * Chaque état est un Behaviour dont onEnd() peut fixer exitValue().
 */
class FSMBehaviour : public CompositeBehaviour {
public:
	FSMBehaviour();
	explicit FSMBehaviour(Agent*);
	virtual ~FSMBehaviour();

	void registerFirstState(Behaviour* b, const std::string& name);
	void registerState     (Behaviour* b, const std::string& name);
	void registerLastState (Behaviour* b, const std::string& name);

	// Transition de <from> vers <to> quand l'état courant retourne exit_value
	void registerTransition       (const std::string& from,
	                               const std::string& to,
	                               int exit_value);
	// Transition par défaut (quand aucune transition spécifique ne correspond)
	void registerDefaultTransition(const std::string& from,
	                               const std::string& to);

	void action()  override;
	bool done()    override;
	void onStart() override;
	void onEnd()   override;

private:
	struct StateEntry {
		Behaviour* beh;
		bool       is_last = false;
	};

	std::map<std::string, StateEntry>                 states_;
	std::map<std::string, std::map<int, std::string>> transitions_;
	std::map<std::string, std::string>                default_transitions_;

	std::string first_state_;
	std::string current_state_;
	bool        finished_ = false;
};

}

#else
namespace gagent {
class Behaviour;
}

#endif /* BEHAVIOUR_H_ */

