/*
 * ReceivedObject.hpp
 */



#include <string>
#include "URL.hpp"
#include "DataTime.hpp"

#ifndef RECEVED_OBJECT_HPP_
#define RECEVED_OBJECT_HPP_

namespace gagent {

class ReceivedObject {
public:
	ReceivedObject();
	virtual ~ReceivedObject();
private:

    Url by;
    Url from;
    DataTime date;
    std::string id;
    std::string via;

};

} /* namespace gagent */

#else
namespace gagent {
	class ReceivedObject;
}
#endif /* RECEVED_OBJECT_HPP_ */
