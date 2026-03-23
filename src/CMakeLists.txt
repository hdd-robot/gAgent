cmake_minimum_required(VERSION 3.15)

find_package(Boost COMPONENTS thread system filesystem REQUIRED)
find_package(Threads REQUIRED)
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Le répertoire de sortie doit exister avant que Bison/Flex s'exécutent
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/messaging)

# Flex/Bison
BISON_TARGET(FipaAclParser
    messaging/fipa_acl.y
    ${CMAKE_CURRENT_BINARY_DIR}/messaging/fipa_acl_parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/messaging/fipa_acl_parser.hpp
    COMPILE_FLAGS "--warnings=all"
)
FLEX_TARGET(FipaAclLexer
    messaging/fipa_acl.l
    ${CMAKE_CURRENT_BINARY_DIR}/messaging/fipa_acl_lexer.cpp
)
ADD_FLEX_BISON_DEPENDENCY(FipaAclLexer FipaAclParser)

set(SRCS
    core/Agent.cpp
    core/AgentCore.cpp
    core/AgentID.cpp
    core/AgentService.cpp
    core/Behaviour.cpp
    core/DFAgentDescription.cpp
    core/DFService.cpp
    messaging/ACLMessage.cpp
    messaging/FipaAclDriver.cpp
    env/Environnement.cpp
    env/VisualAgent.cpp
    comm/CommunicationManager.cpp
    utils/ErrorHandler.cpp
    utils/udp_client_server.cpp
    platform/AMSClient.cpp
    platform/DFClient.cpp
    ${BISON_FipaAclParser_OUTPUTS}
    ${FLEX_FipaAclLexer_OUTPUTS}
)

add_library(gagent SHARED ${SRCS})

target_include_directories(gagent
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/messaging
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/messaging
        ${CMAKE_SOURCE_DIR}/include/gagent/core
        ${CMAKE_SOURCE_DIR}/include/gagent/comm
        ${CMAKE_SOURCE_DIR}/include/gagent/env
        ${CMAKE_SOURCE_DIR}/include/gagent/utils
        ${CMAKE_SOURCE_DIR}/include/gagent/platform
        ${CMAKE_CURRENT_SOURCE_DIR}/platform
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(gagent
    PUBLIC  Boost::system Boost::thread
    PRIVATE Threads::Threads config++ rt
)
