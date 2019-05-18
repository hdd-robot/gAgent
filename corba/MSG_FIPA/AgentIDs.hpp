/*
 * AgentID.hpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */



#include <vector>
#include "AgentID.hpp"

#ifndef AGENTIDS_HPP_
#define AGENTIDS_HPP_

namespace gagent {

class AgentIDs {
public:
	AgentIDs();
	virtual ~AgentIDs();
private:

	std::vector<AgentID> agentIDs;

};

} /* namespace gagent */

#else
namespace gagent {
	class AgentIDs;
}
#endif /* AGENTIDS_HPP_ */
