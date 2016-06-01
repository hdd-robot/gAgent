/*
 * DFAgentDescription.cpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#include "DFAgentDescription.hpp"

namespace gagent {

DFAgentDescription::DFAgentDescription() {

}

void DFAgentDescription::setAgentID(AgentID* aid) {
	agentID = aid;
}

void DFAgentDescription::setDFType(const std::string dftype) {
	DFType = dftype ;

}

void DFAgentDescription::addDFServices(AgentService* srv) {
	DFservicesList.push(srv);

}

DFAgentDescription::~DFAgentDescription() {


}

} /* namespace gagent */

