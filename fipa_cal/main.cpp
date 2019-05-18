
#include <iostream>
#include "ACLMessage.hpp"
#include "Performatives.h"

#include <regex>

using namespace std;

void show_matches(const std::string& in, const std::string& re)
{
    std::smatch m;
    std::regex_search(in, m, std::regex(re));
    if(m.empty()) {
        std::cout << endl << "input=[" << in  << "], regex=[" << re << "]: NO MATCH\n";
    } else {
        std::cout << endl<< "input=[" << in <<  "], regex=[" << re << "]: ";
        std::cout << endl<< "prefix=[" << m.prefix() << "] ";
        for(std::size_t n = 0; n < m.size(); ++n)
            std::cout << endl<< " m[" << n << "]=[" << m[n] << "] ";
        std::cout << endl<< "suffix=[" << m.suffix() << "]\n";
    }
}

int main (int argc, char* argv[]){


 /*
	fipa_cal::ACLMessage* msg = new fipa_cal::ACLMessage();
	msg->setPerformatives(fipa_cal::Performatives::INFORM);

	msg->addRecever("test");
	msg->setSender("toto");
	msg->addRecever("titi");
	msg->setInReplyTo("bid089");
	msg->setContent("((action (agent-identifier :name j) \n \
					 (stream-content movie1234 19)) \n \
					 (B (agent-identifier :name j) \n\
					 (ready customer78)))");
	msg->setLanguage("fipa-sl");

	cout << msg->getTextMsg() << endl;

	*/

	fipa_cal::ACLMessage* msg = new fipa_cal::ACLMessage();
	string msgg = "\
	(inform \
	:sender (agent-identifier :name toto)  \
	:receiver ( set (agent-identifier :name test)(agent-identifier :name titi) ) \
	:content \"((action (agent-identifier :name j) \
	 					 (stream-content movie1234 19)) \
	 					 (B (agent-identifier :name j) \
						 (ready customer78))) \" \
	:in-reply-to bid089 \
	:language fipa-sl \
	) ";

	msg->setTextMsg(msgg);

	cout << msg->getTextMsg() << endl;


	return 0;
}
