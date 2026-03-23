/*
 * AgentService.hpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#include <string>
#include <map>

#ifndef AGENTSERVICE_HPP_
#define AGENTSERVICE_HPP_


namespace gagent {

class AgentService {
public:
	AgentService();
	void setServiceName();
	void setServiceType();
	void setOntologies(const std::string);
	void setLanguages(const std::string);
	void addProperties(const std::string, const std::string);

	virtual ~AgentService();
private :
	std::map<std::string, std::string> DFproprties;
};

} /* namespace gagent */


#else
namespace gagent {
	class AgentService;
}
#endif /* AGENTSERVICE_HPP_ */
