cmake_minimum_required(VERSION 3.1.0)

project(gAgent)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package( Boost COMPONENTS thread system filesystem REQUIRED ) #whatever libs you need
include_directories( ${Boost_INCLUDE_DIRS} )
find_package( Threads REQUIRED)


set( LIBS_TO_LINK
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    Qt5::Widgets
)


set(SRCS
	ACLMessage.cpp 
	AgentCore.cpp 
	AgentID.cpp     
	Behaviour.cpp	
	Agent.cpp 		
	AgentService.cpp 
	DFAgentDescription.cpp 
	udp_client_server.cpp 
	Environnement.cpp  
	EnvironnementGui.cpp  
	VisualAgent.cpp
	MessageParser.cpp
    )
    
set(HEADERS
    ACLMessage.hpp       
    AgentCore.hpp
    AgentID.hpp    
    Behaviour.hpp
	Agent.hpp
	AgentService.hpp
	DFAgentDescription.hpp
	udp_client_server.hpp
	EnvironnementGui.hpp
	Environnement.hpp
	gAgentGui.hpp
	VisualAgent.hpp
	MessageParser.hpp
    )
    
add_library(gagent SHARED ${SRCS} ${HEADERS} )

target_link_libraries( gagent
    ${LIBS_TO_LINK}
)

