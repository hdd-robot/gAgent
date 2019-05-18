/*
 * 	ACLMessage.cpp
 *  Created on: 22 may 2018
 *  Author: Halim Djerroud
 */

#include "ACLMessage.hpp"
#include "Performatives.h"

namespace fipa_cal {


ACLMessage::ACLMessage() {
	this->performatives		= Performatives::NOT_UNDERSTOOD;
	this->clearMsg();

}

ACLMessage::~ACLMessage() {

}

string ACLMessage::getTextMsg() {
	return this->performMessage();
}

void ACLMessage::setTextMsg(const string& msg) {

	string nMsg = msg;

	nMsg.erase(std::remove(nMsg.begin(), nMsg.end(), '\t'), nMsg.end());
	nMsg.erase(std::remove(nMsg.begin(), nMsg.end(), '\n'), nMsg.end());


	this->clearMsg();

	this->setPerformatives(Performatives::INFORM);

	smatch match;

	//content
	regex re(":content( )*\"(.)*\"");
	if (regex_search(nMsg, match,re) && match.size() > 1) {
		string s = match.str(0);
		regex_search(s, match,regex("\"(.|\\n)*\""));
		s = match.str(0);
		s = regex_replace(s,regex("\""),(""));
		this->content = s;
		nMsg = regex_replace(nMsg,re,(""));
	}

	//sender //todo
	re=(":sender( )*\\((.*?)\\)");
	if (regex_search(nMsg, match,re) && match.size() > 1) {
		string s = match.str(0);
		regex_search(s, match,regex("[^(.*:sender( )*agentidentifirt( )*:name)].*[^)]"));
		s = match.str(0);
		this->sender = s;

	}

	//recever //todo
	re=(":recever( )*\"(.)*\"");
	if (regex_search(nMsg, match,re) && match.size() > 1) {
		//this->content = match.str(0);
	}




}

void ACLMessage::addRecever(const string& recever) {
	this->listRecevers.push_back(recever);
}

void ACLMessage::clearListRever() {
	this->listRecevers.clear();
}

bool ACLMessage::removeReceverFromList(string recever) {
	return true;
}

string ACLMessage::getListRecever() {
	string lstRecever;
	vector<string>::iterator it;
	int i = 0;
	for(it = this->listRecevers.begin() ; it != this->listRecevers.end() ; it++ ){
		if(i>0) {lstRecever += ",";}
		lstRecever += *it;
		i++;
	}
	return lstRecever;
}

string ACLMessage::performMessage() {

	string msg;

	msg  = "(" + this->performatives + "\n";
	msg += this->getAclSender();
	msg += this->getAclRecever();
	msg += this->getAclReplayTo() ;
	msg += this->getAclContent() ;
	msg += this->getAclReplyWith() ;
	msg += this->getAclInReplyTo() ;
	msg += this->getAclEnvelope();
	msg += this->getAclLanguage();
	msg += this->getAclOntology();
	msg += this->getAclReplyBy();
	msg += this->getAclProtocol();
	msg += this->getAclConversationId();

	msg += ")";

	return msg;
}

string ACLMessage::getAclSender() {

	if(this->sender != ""){
		return ":sender (agent-identifier :name " + this->sender + ") \n";
	}
	return "";
}

string ACLMessage::getAclRecever() {
	if(this->listRecevers.size()==0){
		return "";
	}
	string msg =  ":receiver ( set " ;

	vector<string>::iterator it;

	for(it = this->listRecevers.begin() ; it != this->listRecevers.end() ; it++ ){
		msg += "(agent-identifier :name " + *it + ")";
	}
	msg += " ) \n";

	return msg;
}

string ACLMessage::getAclReplayTo() {
	if(this->reply_to == ""){
		return "";
	}
	return ":reply-to " + this->reply_to + "\n";
}

string ACLMessage::getAclContent() {
	if(this->content != ""){
			return ":content \""+ this->content+ "\"\n";
	}
	return "";
}

string ACLMessage::getAclReplyWith() {
	if(this->reply_with == ""){
		return "";
	}
	return ":reply-with " + this->reply_with + "\n";
}

string ACLMessage::getAclInReplyTo() {
	if(this->in_reply_to == ""){
		return "";
	}
	return ":in-reply-to " + this->in_reply_to + "\n";
}

string ACLMessage::getAclEnvelope() {
	if(this->envelope == ""){
		return "";
	}
	return ":envelope " + this->envelope + "\n";
}

string ACLMessage::getAclLanguage() {
	if(this->language == ""){
		return "";
	}
	return ":language " + this->language + "\n";
}

string ACLMessage::getAclOntology() {
	if(this->ontology == ""){
		return "";
	}
	return ":ontology " + this->ontology + "\n";
}

string ACLMessage::getAclReplyBy() {
	if(this->reply_by == ""){
		return "";
	}
	return ":reply-by " + this->reply_by + "\n";
}

string ACLMessage::getAclProtocol() {
	if(this->protocol == ""){
		return "";
	}
	return ":protocol " + this->protocol + "\n";
}

void ACLMessage::clearMsg() {
	this->sender			= "";
	this->content			= "";
	this->reply_to			= "";
	this->reply_with		= "";
	this->in_reply_to		= "";
	this->envelope			= "";
	this->language			= "";
	this->ontology			= "";
	this->reply_by			= "";
	this->protocol			= "";
	this->conversation_id	= "";
	this->clearListRever();
}

string ACLMessage::getAclConversationId() {
	if(this->conversation_id == ""){
		return "";
	}
	return ":conversation-id " + this->conversation_id + "\n";
}

} /* namespace fipa_cal */
