/*
 * ACLMessage.hpp
 *
 *  Created on: 23 may 2018
 *      Author: Halim Djerroud
 */

#include <vector>
#include <string>
#include <regex>
#include <iostream>
#include <algorithm>

#ifndef ACLMESSAGE_HPP_
#define ACLMESSAGE_HPP_


namespace fipa_cal {

using namespace std;

class ACLMessage {
public:

	ACLMessage();
	virtual ~ACLMessage();


	string getTextMsg();
	void setTextMsg(const string&);
	void clearMsg();
	void addRecever(const string&);
	void clearListRever();
	bool removeReceverFromList(string);
	string getListRecever();

	string performMessage();

	const string& getContent() const {
		return content;
	}

	void setContent(const string& content) {
		this->content = content;
	}

	const string& getConversationId() const {
		return conversation_id;
	}

	void setConversationId(const string& conversationId) {
		conversation_id = conversationId;
	}

	const string& getEnvelope() const {
		return envelope;
	}

	void setEnvelope(const string& envelope) {
		this->envelope = envelope;
	}

	const string& getInReplyTo() const {
		return in_reply_to;
	}

	void setInReplyTo(const string& inReplyTo) {
		in_reply_to = inReplyTo;
	}

	const string& getLanguage() const {
		return language;
	}

	void setLanguage(const string& language) {
		this->language = language;
	}

	const vector<string>& getListRecevers() const {
		return listRecevers;
	}

	void setListRecevers(const vector<string>& listRecevers) {
		this->listRecevers = listRecevers;
	}

	const string& getOntology() const {
		return ontology;
	}

	void setOntology(const string& ontology) {
		this->ontology = ontology;
	}

	const string& getPerformatives() const {
		return performatives;
	}

	void setPerformatives(const string& performatives) {
		this->performatives = performatives;
	}

	const string& getProtocol() const {
		return protocol;
	}

	void setProtocol(const string& protocol) {
		this->protocol = protocol;
	}

	const string& getReplyBy() const {
		return reply_by;
	}

	void setReplyBy(const string& replyBy) {
		reply_by = replyBy;
	}

	const string& getReplyTo() const {
		return reply_to;
	}

	void setReplyTo(const string& replyTo) {
		reply_to = replyTo;
	}

	const string& getReplyWith() const {
		return reply_with;
	}

	void setReplyWith(const string& replyWith) {
		reply_with = replyWith;
	}

	const string& getSender() const {
		return sender;
	}

	void setSender(const string& sender) {
		this->sender = sender;
	}

private:

	string performatives;
	string sender;
	string content;
	string reply_to;
	string reply_with;
	string in_reply_to;
	string envelope;
	string language;
	string ontology;
	string reply_by;
	string protocol;
	string conversation_id;

	vector <string> listRecevers;

	string getAclSender();
	string getAclRecever();
	string getAclContent();
	string getAclReplayTo();
	string getAclReplyWith();
	string getAclInReplyTo();
	string getAclEnvelope();
	string getAclLanguage();
	string getAclOntology();
	string getAclReplyBy();
	string getAclProtocol();
	string getAclConversationId();



};

} /* namespace gagent */


#else
namespace gagent {
	class ACLMessage;
}
#endif /* ACLMESSAGE_HPP_ */
