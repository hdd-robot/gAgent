cmake_minimum_required(VERSION 3.10)
project(gAgent VERSION 0.8.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_SANITIZERS "Enable address and thread sanitizers" OFF)
option(ENABLE_WARNINGS "Enable extra compiler warnings" ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler flags
if(ENABLE_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(
            -Wall -Wextra -Wpedantic
            -Wno-unused-parameter
            -Wno-missing-field-initializers
        )
    endif()
endif()

# Sanitizers for debugging
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# Find required packages
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread filesystem)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCONFIG REQUIRED libconfig++)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src_agent
    ${Boost_INCLUDE_DIRS}
    ${LIBCONFIG_INCLUDE_DIRS}
)

# Add subdirectories
add_subdirectory(src_agent)

if(BUILD_TESTS)
    add_subdirectory(test_agent)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
install(FILES README.md DESTINATION share/doc/gagent)
install(FILES LICENSE DESTINATION share/doc/gagent)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/gAgentConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Print configuration summary
message(STATUS "")
message(STATUS "gAgent Configuration Summary:")
message(STATUS "  Version:              ${PROJECT_VERSION}")
message(STATUS "  Build type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:         C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build shared libs:    ${BUILD_SHARED_LIBS}")
message(STATUS "  Build tests:          ${BUILD_TESTS}")
message(STATUS "  Build examples:       ${BUILD_EXAMPLES}")
message(STATUS "  Enable sanitizers:    ${ENABLE_SANITIZERS}")
message(STATUS "  Qt5 version:          ${Qt5Core_VERSION}")
message(STATUS "  Boost version:        ${Boost_VERSION}")
message(STATUS "")
