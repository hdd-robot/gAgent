/*
 * DFService.hpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#ifndef DFSERVICE_HPP_
#define DFSERVICE_HPP_

namespace gagent {
#include "Agent.hpp"
#include "DFAgentDescription.hpp"

class DFService {
public:
	DFService();
	static void registerService(const Agent*, const DFAgentDescription*);


	virtual ~DFService();
};

} /* namespace gagent */

#else
namespace gagent {
	class DFService;
}
#endif /* DFSERVICE_HPP_ */
