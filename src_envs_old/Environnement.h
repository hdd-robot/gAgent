/*
 * Environnement.h
 *
 *  Created on: 13 juin 2018
 *      Author: ros
 */



#ifndef ENVS_ENVIRONNEMENT_H_
#define ENVS_ENVIRONNEMENT_H_


#include <map>
#include <string>
#include <iterator>
#include <iostream>
#include <vector>
#include "../src_envs_old/EnvironnementGui.h"
#include "../src_envs_old/VisualAgent.h"

namespace gagent {

class Environnement {
public:
	Environnement();
	virtual ~Environnement();
	virtual void start(bool gui=false) final;

	virtual void clear_nsap() final;
	virtual int  push_nsap()  final;
	virtual int  pull_nsap()  final;
	virtual std::map<int,std::string>* get_nsaps() final; //rang , datetime stamp

	virtual void init_env()=0;
	virtual void link_attribut()=0;
	virtual void event_loop()=0;

	virtual void link_id(std::string _id) final { this->id = _id;}
	virtual void link_name(std::string _name) final { this->name = _name;}
	virtual void link_pos_x(std::string _pos_x) final { this->pos_x = _pos_x;}
	virtual void link_pos_y(std::string _pos_y) final { this->pos_y = _pos_y;}

	virtual void make_agent() final;


	//todo delete these methods
	virtual void initData();
	virtual void printData();



	std::string id		;
	std::string name	;
	std::string shape	;
	std::string	size	;
	std::string	color	;
	std::string pos_x	;
	std::string pos_y	;
	std::string val		;
	std::vector<std::string> vals ;

	std::vector<gagent::VisualAgent*> list_visual_agents;

	int map_width = 600;
	int map_height =300;

private:

	std::map<std::string,std::map<std::string,std::string>> list_attr;
	std::map<std::string,std::map<std::string,std::map<std::string,std::string>>> list_snaps;



};

} /* namespace gagent */

#endif /* ENVS_ENVIRONNEMENT_H_ */














