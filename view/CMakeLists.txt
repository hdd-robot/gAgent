cmake_minimum_required(VERSION 3.15)

find_package(Threads REQUIRED)

add_executable(agentview main.cpp)

add_dependencies(agentview gagent)

target_include_directories(agentview PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(agentview PRIVATE gagent Threads::Threads)

# Symlink agentview → view binary (for PATH convenience)
add_custom_command(TARGET agentview POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:agentview>
        ${CMAKE_CURRENT_BINARY_DIR}/view
    COMMENT "Creating symlink: view -> agentview"
)
