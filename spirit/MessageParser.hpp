/*
 * MessageParser.h
 *
 *  Created on: 21 juin 2018
 *      Author: ros
 */

#ifndef SRC_AGENT_MESSAGEPARSER_HPP_
#define SRC_AGENT_MESSAGEPARSER_HPP_

#include <string>
#include <map>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <iostream>
#include <boost/fusion/include/std_pair.hpp>

namespace gagent {

class MessageParser {
public:

	static bool parsekeyValSeq(std::string,std::map<std::string, std::string> & m);
	virtual ~MessageParser();

private:
	MessageParser();

};

} /* namespace gagent */

#endif /* SRC_AGENT_MESSAGEPARSER_HPP_ */
