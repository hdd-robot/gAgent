/*
 * DFService.cpp — Enregistrement de services auprès du DF de la plateforme
 */

#include "DFService.hpp"
#include <gagent/platform/DFClient.hpp>

namespace gagent {

DFService::DFService() {}

DFService::~DFService() {}

void DFService::registerService(const Agent* ag,
                                 const DFAgentDescription* desc)
{
    if (!ag || !desc) return;

    std::string agentName = ag->getAgentId().getAgentName();
    if (agentName.empty()) agentName = ag->getAgentId().getAgentID();

    const std::string type     = desc->getDFType();
    const std::string svcName  = agentName + "-service";

    platform::DFClient df;
    df.registerService(agentName, type.empty() ? "generic" : type, svcName);
}

} /* namespace gagent */
