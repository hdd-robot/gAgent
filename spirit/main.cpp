#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <iostream>
#include <string>
#include <complex>

namespace acl_messgae
{
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;
	namespace fusion = boost::fusion;

	struct Message_parameter {
		std::vector<std::string> typeOp;
		std::vector<std::string> content;
	};

    struct Message {
		std::vector<std::string> messageType;
    	std::vector<Message_parameter> message_parameter;
    };
}

BOOST_FUSION_ADAPT_STRUCT(
		acl_messgae::Message_parameter,
			(std::vector<std::string>, typeOp)
			(std::vector<std::string>, content)
)

BOOST_FUSION_ADAPT_STRUCT(
		acl_messgae::Message,
			(std::vector<std::string>, messageType)
			(std::vector<acl_messgae::Message_parameter>, message_parameter)
)

namespace acl_messgae {
    template <typename Iterator>
    struct message_parser : qi::grammar<Iterator, Message(), ascii::space_type>
    {
    	message_parser() : message_parser::base_type(msg)
        {
        	using qi::lit;
			using qi::char_;

			msg %= lit('(')  >> 	( lit("accept-proposal")
								| lit("agree")
								| lit("cancel")
								| lit("cfp")
								| lit("confirm")
								| lit("disconfirm")
								| lit("failure")
								| lit("inform")
								| lit("inform-if")
								| lit("inform-ref")
								| lit("not-understood")
								| lit("propagate")
								| lit("propose")
								| lit("proxy")
								| lit("query-ref")
								| lit("refuse")
								| lit("reject-proposal")
								| lit("request")
								| lit("request-when")
								| lit("request-whenever")
								| lit("subscribe") )
							>> *(mp) >> lit (')') ;

			mp 	%=	  ( lit(":sender") 			>>  +(char_ - lit(':')))
					| ( lit(":receiver") 		>>  +(char_ - lit(':')))
					| ( lit(":content")			>>  +(char_ - lit(':')))
					| ( lit(":reply-with") 		>>  +(char_ - lit(':')))
					| ( lit(":reply-by") 		>>	+(char_ - lit(':')))
					| ( lit(":in-reply-to") 	>>  +(char_ - lit(':')))
					| ( lit(":reply-to") 		>>  +(char_ - lit(':')))
					| ( lit(":language") 		>>  +(char_ - lit(':')))
					| ( lit(":encoding") 		>>  +(char_ - lit(':')))
					| ( lit(":ontology") 		>>  +(char_ - lit(':')))
					| ( lit(":protocol") 		>>  +(char_ - lit(':')))
					| ( lit(":conversation-id") >>  +(char_ - lit(':')));

        }

        qi::rule<Iterator, Message(), ascii::space_type> msg;
        qi::rule<Iterator, Message_parameter(), ascii::space_type> mp;


    };
    //]
}


int main() {

    std::cout << "Type [q or Q] to quit\n\n";

    using boost::spirit::ascii::space;
    typedef acl_messgae::message_parser<std::string::const_iterator> message_parser;

    message_parser g; // Our grammar
    std::string str;
    while (getline(std::cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        acl_messgae::Message msg;
        std::string::const_iterator iter = str.begin();
        std::string::const_iterator end = str.end();
        bool r = phrase_parse(iter, end, g, space, msg);

        if (r && iter == end)
        {
            std::cout << boost::fusion::tuple_open('[');
            std::cout << boost::fusion::tuple_close(']');
            std::cout << boost::fusion::tuple_delimiter(", ");

            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
          //  std::cout << "got: " << boost::fusion::as_vector(emp) << std::endl;
            std::cout << "\n-------------------------\n";
        }
        else
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }

//        std::cout << "type : " << msg.messageType << std::endl;
//
//
//        std::vector<std::string>::iterator it;
//        it = msg.message_parameter.begin();
//        std::cout << "parames : " << std::endl;
//        while (it != msg.message_parameter.end()){
//        	std::cout << "p : " << *it << std::endl;
//        	it ++;
//        }



    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}




