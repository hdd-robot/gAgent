find_package(Boost COMPONENTS thread system filesystem REQUIRED)
find_package(Threads REQUIRED)

add_executable(improved_agent improved_agent.cpp)
add_dependencies(improved_agent gagent)

target_include_directories(improved_agent PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/src/messaging
)

target_link_libraries(improved_agent gagent config++ rt)
