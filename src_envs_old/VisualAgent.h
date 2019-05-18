/*
 * VisualAgent.h
 *
 *  Created on: 17 juin 2018
 *      Author: ros
 */



#ifndef SRC_ENVS_OLD_VISUALAGENT_H_
#define SRC_ENVS_OLD_VISUALAGENT_H_

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
	float		size=1.0;
	int			color_r = 0,color_g = 0,color_b = 0;
	float 		pos_x=0;
	float 		pos_y=0;
	std::string val;
	std::vector<std::string> vals;

};

} /* namespace gagent */

#endif /* SRC_ENVS_OLD_VISUALAGENT_H_ */
