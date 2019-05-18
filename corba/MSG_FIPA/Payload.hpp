/*
 * Payload.hpp
 */


#include <string>


#ifndef PAYLOAD_HPP_
#define PAYLOAD_HPP_

namespace gagent {

class Payload {
public:
	Payload();
	virtual ~Payload();
private:

	std::string payload;
};

} /* namespace gagent */

#else
namespace gagent {
	class Payload;
}
#endif /* PAYLOAD_HPP_ */
