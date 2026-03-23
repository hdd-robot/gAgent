find_package(Boost COMPONENTS thread system filesystem REQUIRED)
find_package(Threads REQUIRED)

function(add_gagent_example target source)
    add_executable(${target} ${source})
    add_dependencies(${target} gagent)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/src/messaging
    )
    target_link_libraries(${target} gagent config++ rt)
endfunction()

add_gagent_example(improved_agent      improved_agent.cpp)
add_gagent_example(demo_visualization  demo_visualization.cpp)
add_gagent_example(llm_agent           llm_agent.cpp)
