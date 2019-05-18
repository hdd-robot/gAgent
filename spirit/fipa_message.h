#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <string>
#include <vector>
#include <iostream>

using namespace boost::spirit;

typedef boost::variant<int, bool> int_or_bool;

namespace fipa {


struct MessageParameter{
	std::string type;
};


struct MessageType {
	std::string messageType;
};

struct Message {
public:
	struct MessageType;
	struct MessageParameter;
};



template <typename Iterator, typename Skipper>
struct my_grammar : qi::grammar<Iterator, Message(), Skipper>
{
  my_grammar() : my_grammar::base_type{values}
  {
    value = qi::int_ | qi::bool_;
    values = value >> ',' >> value % ',';
  }

  qi::rule<Iterator, int_or_bool(), Skipper> value;
  qi::rule<Iterator, int_or_bool_values(), Skipper> values;
};


}
