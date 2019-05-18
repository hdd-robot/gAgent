cmake_minimum_required(VERSION 3.1.0)

project(test_gagent)

set (CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(Qt5Core)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package( Boost COMPONENTS thread system filesystem REQUIRED ) #whatever libs you need
include_directories( ${Boost_INCLUDE_DIRS} )
find_package( Threads REQUIRED)



INCLUDE_DIRECTORIES(/home/ros/workspace_cpp/gAgent/src_agent)
LINK_DIRECTORIES(/home/ros/workspace_cpp/gAgent/src_agent/build)


set( LIBS_TO_LINK
    Qt5::Widgets
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    gagent
    config++
)


set(SRCS
	main.cpp
    )
    
set(HEADERS

    )
    
add_executable(main  ${SRCS} ${HEADERS} )

target_link_libraries( main
    ${LIBS_TO_LINK} 
)

