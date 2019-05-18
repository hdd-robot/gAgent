/*
 * VisualAgent.h
 *
 *  Created on: 17 juin 2018
 *      Author: ros
 */



#ifndef SRC_ENVS_VISUALAGENT_H_
#define SRC_ENVS_VISUALAGENT_H_

#include <string>
#include <vector>

namespace gagent {

class VisualAgent {
public:
	VisualAgent();
	VisualAgent(std::string _id);
	VisualAgent(std::string _id, std::string _name);
	VisualAgent(std::string _id, std::string _name, int _pos_x, int _pos_y);
	VisualAgent(std::string _id, std::string _name, int _pos_x, int _pos_y, std::string _val);

	virtual ~VisualAgent();

//private:
	std::string id="";
	std::string name="";
	std::string shape="";
	std::string color="";
	std::string pattern="";
	float		size=1.0;

	float		size_x=1.0;
	float		size_y=1.0;
	float		size_z=1.0;

	float 		pos_x=0;
	float 		pos_y=0;
	std::string val;
	std::vector<std::string> vals;

};

} /* namespace gagent */

#endif /* SRC_ENVS_VISUALAGENT_H_ */
