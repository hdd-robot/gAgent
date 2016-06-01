/*
 * AgentCore.hpp
 *
 *  Created on: 7 août 2014
 *      Author: cyberjunky
 */

#ifndef AGENTCORE_HPP_
#define AGENTCORE_HPP_

namespace gagent {

class AgentCore {
public:

	static void initAgentSystem();

	static void stopAgentSystem();

	static void syncAgentSystem();

private:
	static void cache_signal_handler(int);


};

} /* namespace gagent */

#endif /* AGENTCORE_HPP_ */
