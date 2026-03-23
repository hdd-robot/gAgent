cmake_minimum_required(VERSION 3.10)

project(test_gagent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost COMPONENTS thread system filesystem REQUIRED)
find_package(Threads REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src_agent)
include_directories(${CMAKE_BINARY_DIR}/src_agent)
include_directories(${Boost_INCLUDE_DIRS})

set(LIBS_TO_LINK
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    gagent
    config++
    rt
)

add_executable(main main.cpp)
add_dependencies(main gagent)
target_link_libraries(main ${LIBS_TO_LINK})
