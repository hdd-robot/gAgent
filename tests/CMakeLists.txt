find_package(Boost COMPONENTS thread system filesystem REQUIRED)
find_package(Threads REQUIRED)

# ── Helpers communs ───────────────────────────────────────────────────────────

function(add_gagent_test target source)
    add_executable(${target} ${source})
    add_dependencies(${target} gagent)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/src/messaging
    )
    target_link_libraries(${target} gagent config++ rt)
endfunction()

# ── Tests ─────────────────────────────────────────────────────────────────────

add_gagent_test(test_gagent       main.cpp)
add_gagent_test(two_agents_acl    two_agents_acl.cpp)
add_gagent_test(test_nsap         test_nsap.cpp)
