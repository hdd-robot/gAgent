/*
 * VisualAgent.cpp
 *
 *  Created on: 17 juin 2018
 *      Author: ros
 */

#include "VisualAgent.hpp"

namespace gagent {

VisualAgent::VisualAgent(){}

VisualAgent::VisualAgent(std::string _id):id(_id){}

VisualAgent::VisualAgent(std::string _id, std::string _name):id(_id),name(_name){}

VisualAgent::VisualAgent(std::string _id, std::string _name, int _pos_x, int _pos_y):id(_id),name(_name),pos_x(_pos_x),pos_y(_pos_y){}

VisualAgent::VisualAgent(std::string _id, std::string _name, int _pos_x, int _pos_y, std::string _val):id(_id),name(_name),pos_x(_pos_x),pos_y(_pos_y),val(_val){}


VisualAgent::~VisualAgent() {

}

} /* namespace gagent */
