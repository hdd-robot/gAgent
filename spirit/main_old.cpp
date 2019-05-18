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


//    struct Message {
//		std::vector<boost::variant<std::string,struct Message>> messageType;
//    };

    struct message;

    typedef
        boost::variant<
            boost::recursive_wrapper<message>
          , std::string
        >
    message_node;

    struct message
    {
        std::vector<message_node> children;        // children
    };


    ///////////////////////////////////////////////////////////////////////////
     //  Print out the mini xml tree
     ///////////////////////////////////////////////////////////////////////////
     int const tabsize = 4;

     void tab(int indent)
     {
         for (int i = 0; i < indent; ++i)
             std::cout << ' ';
     }

     struct mini_xml_printer
     {
         mini_xml_printer(int indent = 0)
           : indent(indent)
         {
         }

         void operator()(message const& xml) const;

         int indent;
     };

     struct mini_xml_node_printer : boost::static_visitor<>
     {
         mini_xml_node_printer(int indent = 0)
           : indent(indent)
         {
         }

         void operator()(message const& xml) const
         {
             mini_xml_printer(indent+tabsize)(xml);
         }

         void operator()(std::string const& text) const
         {
             tab(indent+tabsize);
             std::cout << "text: \"" << text << '"' << std::endl;
         }

         int indent;
     };

     void mini_xml_printer::operator()(message const& xml) const
     {

         std::cout << '{' << std::endl;

         BOOST_FOREACH(message_node const& node, xml.children)
         {
             boost::apply_visitor(mini_xml_node_printer(indent), node);
         }

         tab(indent);
         std::cout << '}' << std::endl;
     }

     /*
      * END PRINTER
      *
      */


}


BOOST_FUSION_ADAPT_STRUCT(
	acl_messgae::message,
    (std::vector<acl_messgae::message_node>, children)
)


namespace acl_messgae {
    template <typename Iterator>
    struct message_parser : qi::grammar<Iterator, message(), ascii::space_type>
    {
    	message_parser() : message_parser::base_type(msg)
    	{
    	        	using qi::lit;
    				using qi::char_;
    				using qi::lexeme ;

    				msg = lit('(')  >> 	( lit("accept-proposal")
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

    				mp 	%=	  ( lexeme[":sender"]   >> +qi::alpha  )
    						| ( ":receiver" 		>> +qi::alpha  )
    						| ( ":content"			>> +qi::alpha  )
    						| ( ":reply-with" 		>> +qi::alpha  )
    						| ( ":reply-by" 		>> +qi::alpha  )
    						| ( ":in-reply-to"  	>> +qi::alpha  )
    						| ( ":reply-to" 		>> +qi::alpha  )
    						| ( ":language" 		>> +qi::alpha  )
    						| ( ":encoding" 		>> +qi::alpha  )
    						| ( ":ontology" 		>> +qi::alpha  )
    						| ( ":protocol" 		>> +qi::alpha  )
    						| ( ":conversation-id"  >> +qi::alpha  )  ;

    	        }

        qi::rule<Iterator, message(), ascii::space_type> msg;
        qi::rule<Iterator, message_node(), ascii::space_type> mp;
    };
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

        acl_messgae::message msg;
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
           //std::cout << "got: " << boost::fusion::as_vector(msg) << std::endl;
            std::cout << "\n-------------------------\n";
            acl_messgae::mini_xml_printer printer;
            printer(msg);

        }
        else
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }

      //  std::cout << "type : " << msg.messageType << std::endl;


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

