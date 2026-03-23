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
    add_test(NAME ${target} COMMAND ${target})
endfunction()

# ── Tests ─────────────────────────────────────────────────────────────────────

add_gagent_test(test_gagent        main.cpp)
add_gagent_test(two_agents_acl     two_agents_acl.cpp)
add_gagent_test(test_nsap          test_nsap.cpp)
add_gagent_test(test_acl           test_acl.cpp)
add_gagent_test(test_platform      test_platform.cpp)
add_gagent_test(test_contract_net      test_contract_net.cpp)
add_gagent_test(test_subscribe_notify  test_subscribe_notify.cpp)
