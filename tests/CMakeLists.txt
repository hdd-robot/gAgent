find_package(Boost COMPONENTS thread system filesystem REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

# ── Helpers communs ───────────────────────────────────────────────────────────

function(add_gagent_test target source)
    add_executable(${target} ${source})
    add_dependencies(${target} gagent)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/src/messaging
        ${ZMQ_INCLUDE_DIRS}
    )
    target_link_libraries(${target} gagent config++ ${ZMQ_LIBRARIES})
    add_test(NAME ${target} COMMAND ${target})
endfunction()

# ── Tests ─────────────────────────────────────────────────────────────────────

# test_gagent : démo legacy sans assertions, exclu de CTest
add_executable(test_gagent main.cpp)
add_dependencies(test_gagent gagent)
target_include_directories(test_gagent PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/src/messaging
    ${ZMQ_INCLUDE_DIRS}
)
target_link_libraries(test_gagent gagent config++ ${ZMQ_LIBRARIES})
add_gagent_test(two_agents_acl     two_agents_acl.cpp)
add_gagent_test(test_nsap          test_nsap.cpp)
add_gagent_test(test_acl           test_acl.cpp)
add_gagent_test(test_platform      test_platform.cpp)
add_gagent_test(test_contract_net      test_contract_net.cpp)
add_gagent_test(test_subscribe_notify  test_subscribe_notify.cpp)
add_gagent_test(test_request           test_request.cpp)
add_gagent_test(test_concurrent_send   test_concurrent_send.cpp)
add_gagent_test(test_mock_transport    test_mock_transport.cpp)
