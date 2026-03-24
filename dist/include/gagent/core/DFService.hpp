/*
 * DFService.hpp
 */

#ifndef DFSERVICE_HPP_
#define DFSERVICE_HPP_

#include "Agent.hpp"
#include "DFAgentDescription.hpp"

namespace gagent {

class DFService {
public:
    DFService();
    static void registerService(const Agent*, const DFAgentDescription*);
    virtual ~DFService();
};

} /* namespace gagent */

#endif /* DFSERVICE_HPP_ */
