
#define BOOST_SPIRIT_USE_PHOENIX_V3 1

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <iostream>
#include <string>
#include <complex>
#include <boost/spirit/include/phoenix.hpp>

namespace acl_messgae
{
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;
	namespace fusion = boost::fusion;
	namespace phx = boost::phoenix;



    typedef struct message
    {
    	std::string type_msg_name;
    	std::string receiver;
        std::string sender;
        std::string content;
        std::string reply_with;
        std::string reply_by;
        std::string in_reply_to;
        std::string reply_to;
        std::string language;
        std::string encoding;
        std::string ontology;
        std::string protocol;
		std::string conversation_id;
    }Message;


}

acl_messgae::Message msg;

namespace acl_messgae {
    template <typename Iterator>
    struct message_parser : qi::grammar<Iterator, ascii::space_type>
    {
    	message_parser() :message_parser::base_type(message)
    	{

    				message %= qi::lit('(')  >> ( qi::lit("accept-proposal")	[phx::ref(msg.type_msg_name)="accept-proposal"]
											| qi::lit("agree")				[phx::ref(msg.type_msg_name)="agree"]
											| qi::lit("cancel")				[phx::ref(msg.type_msg_name)="cancel"]
											| qi::lit("cfp")				[phx::ref(msg.type_msg_name)="cfp"]
											| qi::lit("confirm")			[phx::ref(msg.type_msg_name)="confirm"]
											| qi::lit("disconfirm")			[phx::ref(msg.type_msg_name)="disconfirm"]
											| qi::lit("failure")			[phx::ref(msg.type_msg_name)="failure"]
											| qi::lit("inform")				[phx::ref(msg.type_msg_name)="inform"]
											| qi::lit("inform-if")			[phx::ref(msg.type_msg_name)="inform-if"]
											| qi::lit("inform-ref")			[phx::ref(msg.type_msg_name)="inform-ref"]
											| qi::lit("not-understood")		[phx::ref(msg.type_msg_name)="not-understood"]
											| qi::lit("propagate")			[phx::ref(msg.type_msg_name)="propagate"]
											| qi::lit("propose")			[phx::ref(msg.type_msg_name)="propose"]
											| qi::lit("proxy")				[phx::ref(msg.type_msg_name)="proxy"]
											| qi::lit("query-ref")			[phx::ref(msg.type_msg_name)="query-ref"]
											| qi::lit("refuse")				[phx::ref(msg.type_msg_name)="refuse"]
											| qi::lit("reject-proposal")	[phx::ref(msg.type_msg_name)="reject-proposal"]
											| qi::lit("request")			[phx::ref(msg.type_msg_name)="request"]
											| qi::lit("request-when")		[phx::ref(msg.type_msg_name)="request-when"]
											| qi::lit("request-whenever")	[phx::ref(msg.type_msg_name)="request-whenever"]
											| qi::lit("subscribe")			[phx::ref(msg.type_msg_name)="subscribe"]
											)
											>> *(mp)>> qi::lit (')');

					mp 	%=	 			 	  ( qi::lexeme[":sender"]   		>> +qi::alpha[phx::push_back(phx::ref(msg.sender),qi::_1)] )
											| ( qi::lexeme[":receiver"] 		>> +qi::alpha[phx::push_back(phx::ref(msg.receiver),qi::_1)] )
											| ( qi::lexeme[":content"]			>> +qi::alpha[phx::push_back(phx::ref(msg.content),qi::_1)] )
											| ( qi::lexeme[":reply-with"] 		>> +qi::alpha[phx::push_back(phx::ref(msg.reply_with),qi::_1)] )
											| ( qi::lexeme[":reply-by"] 		>> +qi::alpha[phx::push_back(phx::ref(msg.reply_by),qi::_1)] )
											| ( qi::lexeme[":in-reply-to"]  	>> +qi::alpha[phx::push_back(phx::ref(msg.in_reply_to),qi::_1)] )
											| ( qi::lexeme[":reply-to"] 		>> +qi::alpha[phx::push_back(phx::ref(msg.reply_to),qi::_1)] )
											| ( qi::lexeme[":language"] 		>> +qi::alpha[phx::push_back(phx::ref(msg.language),qi::_1)] )
											| ( qi::lexeme[":encoding"] 		>> +qi::alpha[phx::push_back(phx::ref(msg.encoding),qi::_1)] )
											| ( qi::lexeme[":ontology"] 		>> +qi::alpha[phx::push_back(phx::ref(msg.ontology),qi::_1)] )
											| ( qi::lexeme[":protocol"] 		>> +qi::alpha[phx::push_back(phx::ref(msg.protocol),qi::_1)] )
											| ( qi::lexeme[":conversation-id"]  >> +qi::alpha[phx::push_back(phx::ref(msg.conversation_id),qi::_1)] )  ;


    	        }


       qi::rule<Iterator, ascii::space_type> message , mp, word;

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

        std::string::const_iterator iter = str.begin();
        std::string::const_iterator end = str.end();
        bool r = phrase_parse(iter, end, g, space);

        if (r && iter == end)
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
           //std::cout << "got: " << boost::fusion::as_vector(msg) << std::endl;
            std::cout << "\n-------------------------\n";

            std::cout << "type : " << msg.type_msg_name << std:: endl;
            std::cout << "sender : " << msg.sender << std:: endl;
            std::cout << "receiver : " << msg.receiver << std:: endl;
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

