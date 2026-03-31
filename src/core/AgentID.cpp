/*
 * AgentID.cpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#include "AgentID.hpp"
#include <mutex>

namespace gagent {

AgentID::AgentID() {
	agentId = random_string(8);
}

AgentID::~AgentID() {

}

void AgentID::setName(std::string name) {
	agentName = name;

}

void AgentID::setAddresse(std::string adr) {
	agentAdresse = adr;
}

std::string AgentID::getAgentName() {
	return agentName;
}

std::string AgentID::getAgentID() {
	return agentId;
}

std::string gagent::AgentID::random_string(size_t length) {
	static const std::string alphanums = "0123456789"
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	static std::mt19937 rg(std::chrono::system_clock::now().time_since_epoch().count());
	static std::uniform_int_distribution<> pick(0, alphanums.size() - 1);
	static std::mutex gen_mutex;

	std::string s;
	s.reserve(length);

	std::lock_guard<std::mutex> lk(gen_mutex);
	while (length--)
		s += alphanums[pick(rg)];

	return s;
}

} /* namespace gagent */

