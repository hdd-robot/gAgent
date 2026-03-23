/*
 * DFAgentDescription.hpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#include <stack>
#include <string>
#include "AgentService.hpp"
#include "AgentID.hpp"

#ifndef DFAGENTDESCRIPTION_HPP_
#define DFAGENTDESCRIPTION_HPP_

namespace gagent {

class DFAgentDescription {
public:
	DFAgentDescription();
	void setAgentID(AgentID*);
	void setDFType(const std::string);

	void addDFServices(AgentService*);

	virtual ~DFAgentDescription();
private:
	std::stack<AgentService*> DFservicesList;
	std::string DFType;
	AgentID* agentID;

};

} /* namespace gagent */

#else
namespace gagent {
	class DFAgentDescription;
}
#endif /* DFAGENTDESCRIPTION_HPP_ */
