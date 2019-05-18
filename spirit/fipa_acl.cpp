#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <string>
#include <vector>
#include <iostream>

using namespace boost::spirit;

typedef boost::variant<int, bool> int_or_bool;

namespace fipa {





struct AgentIdentifierDef{
	string
};

struct Message_parameterDef {
	std::string param;
	boost::variant<AgentIdentifierDef

					>;
};
struct MessageTypeDef{
	std::string msgType;
};
struct message {
	std::string MessageType;
	std::vector<Message_parameterDef> message_parameter;
};




template<typename Iterator, typename Skipper>
struct fipa_acl_grammar: qi::grammar<Iterator, message(), Skipper> {
	fipa_acl_grammar() :
			fipa_acl_grammar::base_type { Message } {

		using qi::lit;
		using qi::print;
		using qi::int_;
		using qi::float_;
		using qi::char_;
		using qi::digit;



		/**
		 * Lex
		 */

		Word                    = (print - lit('#')) >> *(print);

		StringFipa              = (StringLiteral | ByteLengthEncodedString);

		StringLiteral           = lit('\"') >> *((print - lit('\"') ) | (print - lit("\\\""))) >> lit('\"');

		ByteLengthEncodedString = lit('#') >> +(digit) >> lit("\"") >> (*char_);

		Number                  = int_ | float_;

		URL                     = *(char_);

		DateTimeToken           =  -(Sign) >> Year >> Month >> Day >> lit('T') >> Hour >> Minute >> Second >> MilliSecond >> -(TypeDesignator);

		Year                    = digit >> digit >> digit >> digit;

		Month                   = digit >> digit;

		Day                     = digit >> digit;

		Hour                    = digit >> digit;

		Minute                  = digit >> digit;

		Second                  = digit >> digit;

		MilliSecond             = digit >> digit >> digit;

		TypeDesignator          = *(char_);

		Sign					= (lit('-') | lit('+'));

		DateTimeFipa		 = 	  DateTimeToken;

		UserDefinedParameter =    Word ;

		Expression			 =    WordFipa
								| StringFipa
								| Number
								| DateTimeFipa
								| '(' >> *(Expression) >> ')';

		AgentIdentifier		 =    '(' >> "agent-identifier" >> lit(":name") >> Word >> -(lit(":addresses") >> URLSequence) >> -(lit(":resolvers") >> AgentIdentifierSequence)
											  >>  *(UserDefinedParameter >> Expression) >> ')' ;

		AgentIdentifierSequence = '(' >> lit("sequence") >> *(AgentIdentifier) >> ')';

		AgentIdentifierSet 	 = 	  '(' >> lit("set") >> *(AgentIdentifier) >> ')';

		URLSequence  		 = 	  '(' >> lit("sequence")  >> *(URL) >> ')';

		/**
		 * Lex
		 */

		Message = '(' >> MessageType >> *(MessageParameter) >> ')' ;


		MessageParameter =	  	  lit(":sender") 			>>  AgentIdentifier
								| lit(":receiver") 			>>  AgentIdentifierSet
								| lit(":content")			>>  *(char_)
								| lit(":reply-with") 		>>  Expression
								| lit(":reply-by") 			>>	DateTimeFipa
								| lit(":in-reply-to") 		>>  Expression
								| lit(":reply-to") 			>>  AgentIdentifierSet
								| lit(":language") 			>>  Expression
								| lit(":encoding") 			>>  Expression
								| lit(":ontology") 			>>  Expression
								| lit(":protocol") 			>>  Word
								| lit(":conversation-id") 	>>  Expression
								| UserDefinedParameter	>>  Expression ;


		MessageType =	  	  	  lit("accept-proposal")
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
								| lit("subscribe");

	}

	qi::rule<Iterator, message(), Skipper> Message;
	qi::rule<Iterator, message_parameter(), Skipper> MessageParameter;

	qi::rule<Iterator, std::string(), ascii::space_type> MessageType;

	// todo ....
};

}















