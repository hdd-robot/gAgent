/*
 * Performatives.h
 *
 *  Created on: 22 mai 2018
 *      Author: Halim DJERROUD
 */


#include <string>

#ifndef PERFORMATIVES_H_
#define PERFORMATIVES_H_

namespace fipa_cal {

class Performatives {
public:

	virtual ~Performatives();

	static const std::string ACCEPT_PROPOSAL; 	// Communication de l'accord de l'expéditeur d'effectuer une action qui lui a été préalablement soumise.
	static const std::string AGREE;				// Communication de l'accord de l'expéditeur pour effectuer une action, sans doute dans le futur.
	static const std::string CANCEL;			// Communication de l'annulation de l'accord donnée préalablement par l'expéditeur pour effectuer une action.
	static const std::string CFP; 				// Call for Proposal : Communication par l'expéditeur d'une demande d'effectuer une certaine action.
	static const std::string CONFIRM;			// Communication par l'expéditeur de la confirmation de la validité (selon les règles de l'agent) de la proposition préalablement reçue.
	static const std::string DISCONFIRM;		// Communication par l'expéditeur de la confirmation de la non validité (selon les règles de l'agent) de la proposition préalablement reçue.
	static const std::string FAILURE;			// Communication par l'expéditeur de l'échec d'une action essayée.
	static const std::string INFORM;    		// Communication par l'expéditeur d'une proposition, pensée vrai par celui-ci.
	static const std::string INFORM_IF ;		// Communication par l'expéditeur d'une proposition (pensée vrai par celui-ci), et demande au receveur une confirmation ou une non-confirmation. Macro-action impliquant l'usage de "request".
	static const std::string INFORM_REF; 		// Communication par l'expéditeur d'une demande de l’objet qui correspond à une description envoyée. Macro-action impliquant l'usage de "request".
	static const std::string NOT_UNDERSTOOD;  	// Communication par l'expéditeur d'une non compréhension d'une action effectuée par le destinataire.
	static const std::string PROPAGATE;			// Communication par l'expéditeur d'un message à propager à des agents dont la description est fournie. Le destinataire du message traite le sous-message à propager comme s'il lui était directement destiné et envoie le message "propate" au agent qu'il a identifié
	static const std::string PROPOSE; 			// Communication par l'expéditeur d'une proposition d'action conditionnée à certaines préconditions données.
	static const std::string PROXY;				// Communication par l'expéditeur d'une demande d'une transmission d'un message à des agents dont la description est donnée.
	static const std::string QUERY_REF;			// Communication par l'expéditeur d'une demande par l'expéditeur de l'objet réferrencé par une expression.
	static const std::string REFUSE; 			// Communication par l'expéditeur de son refus d'effectuer une action donnée, et en donne les raisons.
	static const std::string REJECT_PROPOSAL; 	// Communication, pendant une négociation, par l'expéditeur de son refus d'effectuer des actions.
	static const std::string REQUEST;			// Communication par l'expéditeur d'une demande au destinataire d'effectuer une action.
	static const std::string REQUEST_WHEN;		// Communication par l'expéditeur d'une demande, au destinataire, d'effectuer une action quand une proposition donnée devient vrai.
	static const std::string REQUEST_WHENEXER;	// Communication par l'expéditeur d'une demande, au destinataire, d'effectuer une action dès qu'une proposition donnée devient vrai, et à chaque fois que celle-ci redevient vrai.
	static const std::string SUBSCRIBE;			// Communication par l'expéditeur d'une demande d'un objet donnée par une référence envoyé par l'expéditeur, et de renotifier l'agent ayant souscrit dès que l'objet en question change.


private :
	//static class
	Performatives(){}
};

} /* namespace fipa_cal */

#endif /* PERFORMATIVES_H_ */
