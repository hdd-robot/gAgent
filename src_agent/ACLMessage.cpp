/*
 * ACLMessage.cpp
 *
 *  Created on: 15 oct. 2014
 *      Author: cyberjunky
 */

#include "ACLMessage.hpp"

namespace gagent {


const char* ACLMessage::ACCEPT_PROPOSAL	= "accept-proposal";
const char* ACLMessage::AGREE			= "agree";
const char* ACLMessage::CANCEL			= "cancel";
const char* ACLMessage::CFP				= "cfp";
const char* ACLMessage::CONFIRM			= "confirm";
const char* ACLMessage::DISCONFIRM		= "disconfirm";
const char* ACLMessage::FAILURE			= "failure";
const char* ACLMessage::INFORM			= "inform";
const char* ACLMessage::INFORM_IF 		= "inform-if";
const char* ACLMessage::INFORM_REF		= "inform-ref";
const char* ACLMessage::NOT_UNDERSTOOD	= "not-understood";
const char* ACLMessage::PROPAGATE		= "propagate";
const char* ACLMessage::PROPOSE			= "propose";
const char* ACLMessage::PROXY			= "proxy";
const char* ACLMessage::QUERY_REF		= "query-ref";
const char* ACLMessage::REFUSE			= "refuse";
const char* ACLMessage::REJECT_PROPOSAL	= "reject-proposal";
const char* ACLMessage::REQUEST			= "request";
const char* ACLMessage::REQUEST_WHEN	= "request-when";
const char* ACLMessage::REQUEST_WHENEXER= "request-whenever";
const char* ACLMessage::SUBSCRIBE		= "subscribe";

ACLMessage::ACLMessage(const char* aclm) {
	// TODO Auto-generated constructor stub
}

ACLMessage::~ACLMessage() {
	// TODO Auto-generated destructor stub
}

} /* namespace gagent */
