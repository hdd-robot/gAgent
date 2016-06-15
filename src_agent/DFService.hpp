/*
 * DFService.hpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#ifndef DFSERVICE_HPP_
#define DFSERVICE_HPP_
#include "Agent.hpp"

namespace gagent {

class DFService {
public:
	DFService();
	virtual	~DFService();

	const std::string& getInteractionProtocols() const {
		return interactionProtocols;
	}

	void setInteractionProtocols(const std::string& interactionProtocols) {
		this->interactionProtocols = interactionProtocols;
	}

	const std::string& getLanguage() const {
		return language;
	}

	void setLanguage(const std::string& language) {
		this->language = language;
	}

	const std::string& getName() const {
		return name;
	}

	void setName(const std::string& name) {
		this->name = name;
	}

	const std::string& getOwnership() const {
		return ownership;
	}

	void setOwnership(const std::string& ownership) {
		this->ownership = ownership;
	}

	const std::string& getProprites() const {
		return proprites;
	}

	void setProprites(const std::string& proprites) {
		this->proprites = proprites;
	}

	const std::string& getType() const {
		return type;
	}

	void setType(const std::string& type) {
		this->type = type;
	}

private:
	std::string name;
	std::string type;
	std::string ownership;
	std::string interactionProtocols;
	std::string language;
	std::string proprites;
};

} /* namespace gagent */

#else
namespace gagent
{
	class DFService;
}
#endif /* DFSERVICE_HPP_ */
