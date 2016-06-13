/*
 * DFAgentDescription.cpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */
/* namespace gagent */

#include "DFAgentDescription.hpp"

namespace gagent {

DFAgentDescription::DFAgentDescription() {

}

DFAgentDescription::~DFAgentDescription() {


}

void DFAgentDescription::setAgentID(AgentID* aid) {
	agentID = aid;
}

void DFAgentDescription::setDFType(const std::string dftype) {
	DFType = dftype ;

}

void DFAgentDescription::addDFServices(DFService* srv) {
	DFservicesList.push(srv);
}



void DFAgentDescription::setName(std::string nameAgentDescription){
	name = nameAgentDescription;
}

void DFAgentDescription::registerService(const Agent* ag, const DFAgentDescription* sds) {
	// Todo : serialisation ...
}





