/*
 * Environnement.cpp
 *
 *  Created on: 13 juin 2018
 *      Author: ros
 */

#include "../src_envs_old/Environnement.h"


namespace gagent {

Environnement::Environnement() {
	this->id			="";
	this->name			="";
	this->shape			="";
	this->size			="";
	this->color			="";
	this->pos_x			="";
	this->pos_y			="";
	this->val			="";
}

Environnement::~Environnement() {

}

void Environnement::initData() {

	std::map<std::string, std::string> inner;

	inner.clear();
	inner.insert(std::make_pair("id", "agent1"));
	inner.insert(std::make_pair("x", "0"));
	inner.insert(std::make_pair("y", "0"));
	this->list_attr.insert(std::make_pair("agent1", inner));

	inner.clear();
	inner.insert(std::make_pair("id", "agent2"));
	inner.insert(std::make_pair("x", "200"));
	inner.insert(std::make_pair("y", "150"));
	this->list_attr.insert(std::make_pair("agent2", inner));

}

void Environnement::printData() {
	std::map<std::string, std::map<std::string, std::string>>::iterator it;
	std::map<std::string, std::string>::iterator it2;
	for (it = this->list_attr.begin(); it != this->list_attr.end(); it++) {
		std::cout << "------" << std::endl;
		std::cout << "=>" << it->first << std::endl;
		for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			std::cout << it2->first << " : " << it2->second << std::endl;
		}
	}
}



void gagent::Environnement::clear_nsap() {
}

int gagent::Environnement::push_nsap() {
	return 0;
}

int gagent::Environnement::pull_nsap() {
	return 0;
}

void gagent::Environnement::start(bool gui) {

	this->initData();
	this->printData();

	this->init_env();
	this->link_attribut();

	gagent::EnvironnementGui* env = new gagent::EnvironnementGui();
	env->setEnvPtr(this);
	int argc=1;
	char* argv[] = {(char*)""};
	env->createWindow(argc,argv);

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
		std::cout << "=>" << it->first << std::endl;
		for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			std::cout << it2->first << " : " << it2->second << std::endl;

			if(it2->first == this->id)			{ v_agent->id		= it2->second ;}
			else if(it2->first == this->name)	{ v_agent->name		= it2->second ;}
			else if(it2->first == this->pos_x)	{try{ v_agent->pos_x= std::stof(it2->second);}catch(...){v_agent->pos_x=0;}}
			else if(it2->first == this->pos_y)	{try{ v_agent->pos_y= std::stof(it2->second);}catch(...){v_agent->pos_x=0;}}
			else if(it2->first == this->val)	{ v_agent->val		= it2->second ;}
			else if(it2->first == this->shape)	{ v_agent->shape	= it2->second ;}
			else if(it2->first == this->size)	{try{v_agent->size	=std::stof(it2->second);}catch(...){v_agent->pos_x=0;}}
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


} /* namespace gagent */















