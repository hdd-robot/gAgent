cmake_minimum_required(VERSION 3.10)

# Find packages
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

include_directories(${CMAKE_SOURCE_DIR}/src_agent)
include_directories(${CMAKE_BINARY_DIR}/src_agent)

# Improved agent example
add_executable(improved_agent improved_agent.cpp)
add_dependencies(improved_agent gagent)
target_link_libraries(improved_agent
    gagent
    Qt5::Core
    Qt5::Widgets
    ${Boost_LIBRARIES}
    config++
    pthread
    rt
)

# Install examples
install(TARGETS improved_agent DESTINATION bin/examples)
