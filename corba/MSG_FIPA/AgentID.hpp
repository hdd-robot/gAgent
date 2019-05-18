/*
 * AgentID.hpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#include <chrono>
#include <random>
#include <string>
#include <vector>
#include "Property.hpp"
#include "URL.hpp"

#ifndef AGENTID_HPP_
#define AGENTID_HPP_

namespace gagent {

class AgentID {
public:
	AgentID();
	virtual ~AgentID();
private:

	std::string name;
	std::vector<Url> addresses;
	std::vector<AgentID> resolvers;
	std::vector<Property> userDefinedProperties;

};

} /* namespace gagent */

#else
namespace gagent {
	class AgentID;
}
#endif /* AGENTID_HPP_ */
