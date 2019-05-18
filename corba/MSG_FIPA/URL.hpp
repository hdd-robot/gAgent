/*
 * AgentID.hpp
 */

#include <string>


#ifndef URL_HPP_
#define URL_HPP_

namespace gagent {

class Url {
public:
	Url();
	virtual ~Url();
private:

	std::string url;
};

} /* namespace gagent */

#else
namespace gagent {
	class Url;
}
#endif /* URL_HPP_ */
