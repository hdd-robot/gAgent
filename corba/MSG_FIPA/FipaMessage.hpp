/*
 * FipaMessage.hpp
 */

#include "Envelopes.hpp"
#include "Payload.hpp"

#ifndef FIPA_MESSAGE_HPP_
#define FIPA_MESSAGE_HPP_

namespace gagent {

class FipaMessage {
public:
	FipaMessage();
	virtual ~FipaMessage();
private:

	Envelopes messageEnvelopes;
	Payload messageBody;

};

} /* namespace gagent */

#else
namespace gagent {
	class FipaMessage;
}
#endif /* FIPA_MESSAGE_HPP_ */
