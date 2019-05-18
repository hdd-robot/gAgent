cmake_minimum_required(VERSION 3.1.0)

project(gagentinterface)

set (CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)


find_package(Qt5Core)
find_package(Qt5Widgets CONFIG REQUIRED)


set( LIBS_TO_LINK
    Qt5::Widgets
)


set(SRCS
	Environnement.cpp  
	EnvironnementGui.cpp  
	main.cpp    
	VisualAgent.cpp
    )
    
set(HEADERS
	EnvironnementGui.h  
	Environnement.h  
	gAgentGui.h  
	VisualAgent.h
    )
    
add_executable(main3  ${SRCS} ${HEADERS} )

target_link_libraries( main3
    ${LIBS_TO_LINK} 
)

