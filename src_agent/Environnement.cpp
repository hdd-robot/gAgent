/*

/* Environnement.cpp
 *
 *  Created on: 13 juin 2018
 *      Author: ros
 */

#include "Environnement.hpp"


namespace gagent {

Environnement::Environnement() {
	this->id			="id";
	this->name			="name";
	this->size			="size";
	this->size_x		="size_x";
	this->size_y		="size_y";
	this->size_z		="size_z";
	this->color			="color";
	this->pos_x			="pos_x";
	this->pos_y			="pos_y";
	this->val			="val";
	this->pattern		="pattern";

	bool dbg = 1;
	if (dbg > 0) {
		this->udpMonitor = new udp_client_server::udp_client("127.0.0.1", 40013);
	}


}

Environnement::~Environnement() {

}

void Environnement::readDataFromQueueMsg() {
	char* mq_name = (char*)"/envqueuemsg";
	mqd_t  mq;

	int    taille = 1000;
	int    max_msg = 5;
	char*  buffer = (char*)malloc(taille);

	struct mq_attr attr;
	attr.mq_flags   = 0;
	attr.mq_maxmsg  = max_msg;
	attr.mq_msgsize = taille;
	attr.mq_curmsgs = 0;

	mq = mq_open(mq_name, O_RDONLY | O_CREAT , 0666 , &attr);
	perror("mq_open ");
	if(mq == (mqd_t)-1){
		std::cout << "Error create Message Queue " << std::endl; // todo : send monitor
		return;
	}

	std::map<std::string,std::string> m;
	std::map<std::string,std::string>::iterator it_attr;
	std::map<std::string,std::map<std::string,std::string>>::iterator it_list_attr;
	int ret;
	while (true) {
		//std::cout << "begin receve from agents " << std::endl;
		ret = mq_receive(mq, buffer, taille, NULL);
		if(ret < 0){
			perror("mq_receive ");
		}

		std::string sbuffer(buffer);
		m.clear();

		if (MessageParser::parsekeyValSeq(sbuffer,m)){
			//std::cout << "receved message OK  <" << sbuffer << ">"<< std::endl;

			it_attr = m.find(this->id);

			if(it_attr != m.end()){
				std::string agent_id = it_attr->second;

				it_list_attr = this->list_attr.find(agent_id);

				if (it_list_attr != this->list_attr.end()){
					it_list_attr->second = m;
				}
				else{
					this->list_attr.insert(std::make_pair(agent_id, m));
				}
			}
			//std::cout << "end receved message OK  : " << sbuffer << std::endl;
		}
		else{
			std::cout << "receved message not conform  : " << sbuffer << std::endl; // todo : send monitor
		}
	}
}

//void Environnement::initData() {
//
//	std::map<std::string, std::string> inner;
//
//	inner.clear();
//	inner.insert(std::make_pair("id", "agent1"));
//	inner.insert(std::make_pair("x", "0"));
//	inner.insert(std::make_pair("y", "0"));
//	this->list_attr.insert(std::make_pair("agent1", inner));
//
//	inner.clear();
//	inner.insert(std::make_pair("id", "agent2"));
//	inner.insert(std::make_pair("x", "200"));
//	inner.insert(std::make_pair("y", "150"));
//	this->list_attr.insert(std::make_pair("agent2", inner));
//
//}

//void Environnement::printData() {
//	std::map<std::string, std::map<std::string, std::string>>::iterator it;
//	std::map<std::string, std::string>::iterator it2;
//	for (it = this->list_attr.begin(); it != this->list_attr.end(); it++) {
//		std::cout << "------" << std::endl;
//		std::cout << "=>" << it->first << std::endl;
//		for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
//			std::cout << it2->first << " : " << it2->second << std::endl;
//		}
//	}
//}



void gagent::Environnement::clear_nsap() {
}

int gagent::Environnement::push_nsap() {
	return 0;
}

int gagent::Environnement::pull_nsap() {
	return 0;
}

void gagent::Environnement::start(bool gui,unsigned int timer_val) {

//	this->initData();
//	this->printData();

	this->init_env();
	this->link_attribut();

	gagent::EnvironnementGui* env = new gagent::EnvironnementGui();
	env->setEnvPtr(this);
	int argc=1;
	char* argv[] = {(char*)""};
	env->createWindow(argc,argv,gui,timer_val);

}

std::map<int, std::string> *gagent::Environnement::get_nsaps() {
	std::map<int, std::string>* m= nullptr;
	m->insert(std::make_pair(1,"test"));
	return m;
}

void gagent::Environnement::make_agent() {

	std::map<std::string, std::map<std::string, std::string>>::iterator it;
	std::map<std::string, std::string>::iterator it2;

	for (int i = 0; i < this->list_visual_agents.size(); i++) {
		delete this->list_visual_agents[i];
	}

	this->list_visual_agents.clear();

	for (it = this->list_attr.begin(); it != this->list_attr.end(); it++) {
		gagent::VisualAgent* v_agent = new gagent::VisualAgent();
		//std::cout << "=> enregistrement : " << it->first << std::endl;
		for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			//std::cout << "velue : " << it2->first << " : " << it2->second << std::endl;
			if(it2->first == this->id)			{ v_agent->id		= it2->second ;}
			else if(it2->first == this->name)	{ v_agent->name		= it2->second ;}
			else if(it2->first == this->pos_x)	{try{ v_agent->pos_x= std::stof(it2->second);}catch(...){v_agent->pos_x=0;}}
			else if(it2->first == this->pos_y)	{try{ v_agent->pos_y= std::stof(it2->second);}catch(...){v_agent->pos_y=0;}}
			else if(it2->first == this->val)	{ v_agent->val		= it2->second ;}
			else if(it2->first == this->shape)	{ v_agent->shape	= it2->second ;}
			else if(it2->first == this->color)	{ v_agent->color	= it2->second ;}
			else if(it2->first == this->pattern){ v_agent->pattern	= it2->second ;}
			else if(it2->first == this->size)	{try{v_agent->size	= std::stof(it2->second);}catch(...){v_agent->size=0;}}
			else if(it2->first == this->size_x)	{try{v_agent->size_x= std::stof(it2->second);}catch(...){v_agent->size_x=0;}}
			else if(it2->first == this->size_y)	{try{v_agent->size_y= std::stof(it2->second);}catch(...){v_agent->size_y=0;}}
			else if(it2->first == this->size_z)	{try{v_agent->size_z= std::stof(it2->second);}catch(...){v_agent->size_z=0;}}
			else 								{ v_agent->vals.push_back(it2->second);}
		}

		if (v_agent->id ==""){
			delete (v_agent);
			continue;
		}

		if (v_agent->shape =="")	{ v_agent->shape = "circle";}
		if (v_agent->size  == 0) 	{ v_agent->size  = 5.0;}

		this->list_visual_agents.push_back(v_agent);

	}


}

int gagent::Environnement::sendMsgMonitor(std::string msg) {
	msg = "Environnement -> " + msg;
	this->udpMonitor->send((char*) msg.c_str(), BUFLEN);
	return 0;
}


} /* namespace gagent */















