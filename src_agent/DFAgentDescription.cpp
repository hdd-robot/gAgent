/*
 * DFAgentDescription.cpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#include "DFAgentDescription.hpp"

namespace gagent
{

  DFAgentDescription::DFAgentDescription ()
  {

  }

  DFAgentDescription::~DFAgentDescription ()
  {

  }

  void DFAgentDescription::setAgentID (AgentID* aid){
    this->agentID = aid;
  }

  void DFAgentDescription::setDFType (const std::string dftype){
    this->DFType = dftype;

  }

  void DFAgentDescription::addDFServices (DFService* srv){
    this->DFservicesList.push_back(srv);
  }

  void DFAgentDescription::setName (std::string nameAgentDescription){
    this->name = nameAgentDescription;
  }

  void DFAgentDescription::registerService (const Agent* ag, const DFAgentDescription* sds){
    // Todo : serialisation ...
    std::cout << "register" << std::endl;
  }

}/* namespace gagent */

