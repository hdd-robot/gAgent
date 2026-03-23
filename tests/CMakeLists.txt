find_package(Boost COMPONENTS thread system filesystem REQUIRED)
find_package(Threads REQUIRED)

add_executable(test_gagent main.cpp)
add_dependencies(test_gagent gagent)

target_include_directories(test_gagent PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/src/messaging
)

target_link_libraries(test_gagent gagent config++ rt)
