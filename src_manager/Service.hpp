/*
 * Service.hpp
 *
 *  Created on: 7 juin 2016
 *      Author: socrate
 */



#ifndef SRC_MANAGER_SERVICE_HPP_
#define SRC_MANAGER_SERVICE_HPP_

using namespace std;

class Service {
public:
	Service();
	virtual ~Service();

private:
	string type;
	string name;
	string ownership;
	string interactionProtocols;
	string language;
	string proprites;
};

#endif /* SRC_MANAGER_SERVICE_HPP_ */
