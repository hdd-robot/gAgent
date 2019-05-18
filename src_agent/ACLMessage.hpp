/*
 * ACLMessage.hpp
 *
 *  Created on: 15 oct. 2014
 *      Author: cyberjunky
 */

#include <vector>

#ifndef ACLMESSAGE_HPP_
#define ACLMESSAGE_HPP_

#include "AgentID.hpp"


namespace gagent {

class ACLMessage {
public:
	ACLMessage(const char*);
	virtual ~ACLMessage();

	void addReceiver(AgentID);
	void setLanguage(char*);
	void setOntology(char*);
	void setContent(char*);
	void setPerformative();
	void createReply();
	void getTextMsg();

	static const char* ACCEPT_PROPOSAL; // Communication de l'accord de l'expéditeur d'effectuer une action qui lui a été préalablement soumise.
	static const char* AGREE;			// Communication de l'accord de l'expéditeur pour effectuer une action, sans doute dans le futur.
	static const char* CANCEL;			// Communication de l'annulation de l'accord donnée préalablement par l'expéditeur pour effectuer une action.
	static const char* CFP; 			// Call for Proposal : Communication par l'expéditeur d'une demande d'effectuer une certaine action.
	static const char* CONFIRM;			// Communication par l'expéditeur de la confirmation de la validité (selon les règles de l'agent) de la proposition préalablement reçue.
	static const char* DISCONFIRM;		// Communication par l'expéditeur de la confirmation de la non validité (selon les règles de l'agent) de la proposition préalablement reçue.
	static const char* FAILURE;			// Communication par l'expéditeur de l'échec d'une action essayée.
	static const char* INFORM;    		// Communication par l'expéditeur d'une proposition, pensée vrai par celui-ci.
	static const char* INFORM_IF ;		// Communication par l'expéditeur d'une proposition (pensée vrai par celui-ci), et demande au receveur une confirmation ou une non-confirmation. Macro-action impliquant l'usage de "request".
	static const char* INFORM_REF; 		// Communication par l'expéditeur d'une demande de l’objet qui correspond à une description envoyée. Macro-action impliquant l'usage de "request".
	static const char* NOT_UNDERSTOOD;  // Communication par l'expéditeur d'une non compréhension d'une action effectuée par le destinataire.
	static const char* PROPAGATE;		// Communication par l'expéditeur d'un message à propager à des agents dont la description est fournie. Le destinataire du message traite le sous-message à propager comme s'il lui était directement destiné et envoie le message "propate" au agent qu'il a identifié
	static const char* PROPOSE; 		// Communication par l'expéditeur d'une proposition d'action conditionnée à certaines préconditions données.
	static const char* PROXY;			// Communication par l'expéditeur d'une demande d'une transmission d'un message à des agents dont la description est donnée.
	static const char* QUERY_REF;		// Communication par l'expéditeur d'une demande par l'expéditeur de l'objet réferrencé par une expression.
	static const char* REFUSE; 			// Communication par l'expéditeur de son refus d'effectuer une action donnée, et en donne les raisons.
	static const char* REJECT_PROPOSAL; // Communication, pendant une négociation, par l'expéditeur de son refus d'effectuer des actions.
	static const char* REQUEST;			// Communication par l'expéditeur d'une demande au destinataire d'effectuer une action.
	static const char* REQUEST_WHEN;	// Communication par l'expéditeur d'une demande, au destinataire, d'effectuer une action quand une proposition donnée devient vrai.
	static const char* REQUEST_WHENEXER;// Communication par l'expéditeur d'une demande, au destinataire, d'effectuer une action dès qu'une proposition donnée devient vrai, et à chaque fois que celle-ci redevient vrai.
	static const char* SUBSCRIBE;		// Communication par l'expéditeur d'une demande d'un objet donnée par une référence envoyé par l'expéditeur, et de renotifier l'agent ayant souscrit dès que l'objet en question change.

private:
	std::vector <AgentID*> listRecevers;

};

} /* namespace gagent */


#else
namespace gagent {
	class ACLMessage;
}
#endif /* ACLMESSAGE_HPP_ */
