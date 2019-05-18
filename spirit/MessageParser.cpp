/*
 * MessageParser.cpp
 *
 *  Created on: 21 juin 2018
 *      Author: ros
 */

#include "MessageParser.hpp"

namespace gagent {

namespace msgKeyVal
{
    namespace qi = boost::spirit::qi;

    template <typename Iterator>
    struct key_value_sequence
      : qi::grammar<Iterator, std::map<std::string, std::string> ()>
    {
        key_value_sequence()
          : key_value_sequence::base_type(query)
        {
            query =  pair >> *((qi::lit(';')) >> pair);
            pair  =  key >> -(':' >> value);
            key   =  qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
            value = +qi::char_("a-zA-Z_0-9");
        }

        qi::rule<Iterator, std::map<std::string, std::string> ()> query;
        qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
        qi::rule<Iterator, std::string()> key, value;
    };
}



bool MessageParser::parsekeyValSeq(std::string message, std::map<std::string, std::string> & m) {

	namespace qi = boost::spirit::qi;
	std::string::iterator begin = message.begin();
	std::string::iterator end = message.end();

	msgKeyVal::key_value_sequence<std::string::iterator> p;
	m.clear();

	if (!qi::parse(begin, end, p, m))
	{
		m.clear();
		return false;
	}
//	else
//	{
//		std::cout << "-------------------------------- \n";
//		std::cout << "Parsing succeeded, found entries:\n";
//		std::map<std::string, std::string> ::iterator end = m.end();
//		for (std::map<std::string, std::string> ::iterator it = m.begin(); it != end; ++it)
//		{
//			std::cout << (*it).first;
//			if (!(*it).second.empty())
//				std::cout << "=" << (*it).second;
//			std::cout << std::endl;
//		}
//		std::cout << "---------------------------------\n";
//	}
	return true;
}

MessageParser::MessageParser() {

}

MessageParser::~MessageParser() {
}

} /* namespace gagent */
