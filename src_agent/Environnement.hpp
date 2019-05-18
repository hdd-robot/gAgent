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
#include <mqueue.h>
#include <thread>
#include "EnvironnementGui.hpp"
#include "VisualAgent.hpp"
#include "MessageParser.hpp"
#include "udp_client_server.hpp"

#define BUFLEN 1024  				// Max length of buffer

namespace gagent {

class Environnement {
public:
	Environnement();
	virtual ~Environnement();
	virtual void start(bool gui, unsigned int timer_val) final;

	virtual void clear_nsap() final;
	virtual int  push_nsap()  final;
	virtual int  pull_nsap()  final;
	virtual std::map<int,std::string>* get_nsaps() final; //rang , datetime stamp

	virtual void init_env(){};
	virtual void link_attribut(){};
	virtual void event_loop(){};

	virtual void link_id(std::string _id) 			final { this->id = _id;}
	virtual void link_name(std::string _name) 		final { this->name = _name;}
	virtual void link_pos_x(std::string _pos_x) 	final { this->pos_x = _pos_x;}
	virtual void link_pos_y(std::string _pos_y) 	final { this->pos_y = _pos_y;}
	virtual void link_color(std::string _color) 	final { this->color = _color;}
	virtual void link_shape(std::string _shape) 	final { this->shape = _shape;}
	virtual void link_size(std::string _size) 		final { this->size  = _size;}
	virtual void link_size_x(std::string _size_x) 	final { this->size_x= _size_x;}
	virtual void link_size_y(std::string _size_y) 	final { this->size_y= _size_y;}
	virtual void link_size_z(std::string _size_z) 	final { this->size_z= _size_z;}
	virtual void link_val(std::string _val) 		final { this->val = _val;}

	virtual void make_agent() final;

	virtual void readDataFromQueueMsg();
	//todo delete these methods
//	virtual void initData();
//	virtual void printData();

	int sendMsgMonitor(std::string);

	std::string id		;
	std::string name	;
	std::string shape	;
	std::string	size	;
	std::string	size_x	;
	std::string	size_y	;
	std::string	size_z	;
	std::string	color	;
	std::string pos_x	;
	std::string pos_y	;
	std::string val		;
	std::string pattern ;
	std::vector<std::string> vals ;

	std::vector<gagent::VisualAgent*> list_visual_agents;

	int map_width  = 600;
	int map_height = 300;

private:

	std::map<std::string,std::map<std::string,std::string>> list_attr;
	std::map<std::string,std::map<std::string,std::map<std::string,std::string>>> list_snaps;
	udp_client_server::udp_client* udpMonitor = nullptr;


};

} /* namespace gagent */

#endif /* ENVS_ENVIRONNEMENT_H_ */














