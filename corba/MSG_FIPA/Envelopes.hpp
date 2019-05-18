/*
 * Envelopes.hpp
 */



#ifndef ENVELOPES_HPP_
#define ENVELOPES_HPP_


#include "Envelope.hpp"

#include <vector>
#include <string>

namespace gagent {

class Envelopes {
public:
	Envelopes();
	virtual ~Envelopes();
private:

	std::vector<Envelope> envelopes;

};

} /* namespace gagent */

#else
namespace gagent {
	class Envelopes;
}
#endif /* ENVELOPES_HPP_ */
