/*
 * Envelope.hpp
 */



#ifndef ENVELOPE_HPP_
#define ENVELOPE_HPP_


#include "AgentID.hpp"
#include "AgentIDs.hpp"
#include "DataTime.hpp"
#include "RecevedObject.hpp"
#include "Property.hpp"
#include <vector>
#include <string>

namespace gagent {

class Envelope {
public:
	Envelope();
	virtual ~Envelope();
private:

    AgentIDs to;
    AgentID from;
    std::string comments;
    std::string aclRepresentation;
    long payloadLength;
    std::string payloadEncoding;
    DataTime date;
    std::string encrypted;
    AgentIDs intendedReceiver;
    ReceivedObject received;
	std::vector<Property> transportBehaviour;
	std::vector<Property> userDefinedProperties; // user-defined properties

};

} /* namespace gagent */

#else
namespace gagent {
	class Envelope;
}
#endif /* ENVELOPE_HPP_ */
