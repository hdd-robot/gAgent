/*
 * AgentID.hpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#include <chrono>
#include <random>
#include <string>

#ifndef AGENTID_HPP_
#define AGENTID_HPP_

namespace gagent {

class AgentID {
public:
	AgentID();

 	void setName(std::string);
 	void setAddresse(std::string);
	std::string getAgentName();
	std::string getAgentID();

	virtual ~AgentID();
private:

	std::string agentAdresse;
	std::string agentName;
	std::string agentId;

	std::string random_string(size_t length);

};

} /* namespace gagent */


#else
namespace gagent {
	class AgentID;
}
#endif /* AGENTID_HPP_ */
