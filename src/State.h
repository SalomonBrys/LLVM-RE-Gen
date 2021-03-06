/*
 * State.h
 *
 *  Created on: 14 oct. 2010
 *      Author: So
 */

#ifndef STATE_H_
#define STATE_H_

#include <queue>
#include <map>
#include <vector>
#include <set>
#include <ostream>

struct IState;
struct State;
struct StateReplicator;
struct DState;

typedef std::queue<StateReplicator*> ReplicatorQueue;
typedef std::vector<State*> StateVector;
typedef std::multimap<int, IState*> StateTransitions;
typedef std::pair<StateTransitions::const_iterator, StateTransitions::const_iterator> ConstStateTransitionsRange;

struct StateHelper
{
	StateHelper() {}
	StateVector states;
	ReplicatorQueue queue;

	void clear();
};
std::ostream & operator << (std::ostream & os, const StateHelper & node);


typedef std::map<int, int> DStateTransitions;

struct IState
{
	virtual ~IState() {}

	void setFinal(bool f)
	{
		_setFinal(f);
		for (std::set<IState *>::iterator it = _repFinal.begin(); it != _repFinal.end(); ++it)
			(*it)->setFinal(f);
	}

	void addReplicatedFinal(IState * state) { _repFinal.insert(state); }

	virtual bool Final(void) const = 0;
	virtual const StateTransitions & Transitions(void) const = 0;
	virtual void addTransition(int c, IState* s) = 0;
	virtual int Name(void) const  = 0;

protected:
	virtual void _setFinal(bool) = 0;

private:
	std::set<IState *> _repFinal;
};

struct State : public IState
{
	State(StateHelper & helper) : _helper(helper), _name(helper.states.size()), _final(true) { helper.states.push_back(this); }
	virtual ~State() { _helper.states[_name] = 0; }

	virtual bool Final(void) const { return _final; }
	virtual const StateTransitions & Transitions(void) const { return _transitions; }
	virtual void addTransition(int c, IState* s) { _transitions.insert(StateTransitions::value_type(c, s)); }
	virtual int Name(void) const { return _name; }

protected:
	virtual void _setFinal(bool is) { _final = is; }

private:
	StateHelper & _helper;

	const int	_name;
	bool _final;
	StateTransitions _transitions;

	State(const State &);
	State & operator = (const State &);
};

struct StateReplicator : public IState
{
	StateReplicator(IState * orig, IState * copy, StateHelper & helper) : _orig(orig), _copy(copy) { helper.queue.push(this); }
	virtual ~StateReplicator() {}

	virtual bool Final(void) const { return _orig->Final(); }
	virtual const StateTransitions & Transitions(void) const { return _orig->Transitions(); }
	virtual void addTransition(int c, IState* s) { _orig->addTransition(c, s); _copy->addTransition(c, s); }
	virtual int Name(void) const { return _orig->Name(); }

protected:
	virtual void _setFinal(bool is) { _orig->setFinal(is); _copy->setFinal(is); }

private:
	IState * _orig;
	IState * _copy;

	StateReplicator(const State &);
	State & operator = (const State &);
};

struct DState
{
	DStateTransitions transitions;
	bool final;
};

struct DFSM : public std::vector<DState*> // = Determinist Finite State Machine
{
	virtual ~DFSM()
	{
		for (DFSM::iterator it = begin(); it != end(); ++it)
			if (*it)
				delete *it;
	}
};
std::ostream & operator << (std::ostream & os, const DFSM & dfsm);

void determine(StateVector &, DFSM &, bool stopAtFirstMatch);
void reduce(DFSM &);

#endif /* STATE_H_ */
