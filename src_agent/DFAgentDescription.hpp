/*
 * DFAgentDescription.hpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#include <stack>
#include <string>

#include "DFService.hpp"
#include "AgentID.hpp"
#include "Agent.hpp"

#ifndef DFAGENTDESCRIPTION_HPP_
#define DFAGENTDESCRIPTION_HPP_

namespace gagent {

class DFAgentDescription {
public:
	DFAgentDescription();
	void setName(std::string);
	void setAgentID(AgentID*);
	void setDFType(const std::string);

	static void registerService(const Agent*, const DFAgentDescription*);

	void addDFServices(DFService*);

	virtual ~DFAgentDescription();
private:
	std::stack<DFService*> DFservicesList;
	std::string DFType;
	std::string name;
	AgentID* agentID;



};

} /* namespace gagent */

#else
namespace gagent {
	class DFAgentDescription;
}
#endif /* DFAGENTDESCRIPTION_HPP_ */
