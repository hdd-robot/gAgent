/*
 *
 *
 * */

#include <string>

#ifndef PROPERTY_HPP_
#define PROPERTY_HPP_

namespace gagent {

class Property {
public:
	Property();
	virtual ~Property();
private:

	std::string keyword ;
	std::string value ;
};

} /* namespace gagent */


#else
namespace gagent {
	class Property;
}
#endif /* PROPERTY_HPP_ */
