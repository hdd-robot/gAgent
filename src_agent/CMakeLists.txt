cmake_minimum_required(VERSION 3.10)

project(gAgent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(BUILD_GUI "Build Qt GUI visualization (requires Qt5)" OFF)

find_package(Boost COMPONENTS thread system filesystem REQUIRED)
find_package(Threads REQUIRED)
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

# ── GUI optionnel ─────────────────────────────────────────────────────────────

if(BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Widgets CONFIG REQUIRED)
    add_compile_definitions(BUILD_GUI)
    message(STATUS "GUI Qt5: ON")
else()
    message(STATUS "GUI Qt5: OFF (BUILD_GUI=OFF)")
endif()

# ── Flex / Bison ──────────────────────────────────────────────────────────────

BISON_TARGET(FipaAclParser
    fipa_acl.y
    ${CMAKE_CURRENT_BINARY_DIR}/fipa_acl_parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/fipa_acl_parser.hpp
    COMPILE_FLAGS "--warnings=all"
)

FLEX_TARGET(FipaAclLexer
    fipa_acl.l
    ${CMAKE_CURRENT_BINARY_DIR}/fipa_acl_lexer.cpp
)

ADD_FLEX_BISON_DEPENDENCY(FipaAclLexer FipaAclParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# ── Sources ───────────────────────────────────────────────────────────────────

set(SRCS
    ACLMessage.cpp
    AgentCore.cpp
    AgentID.cpp
    Behaviour.cpp
    Agent.cpp
    AgentService.cpp
    DFAgentDescription.cpp
    udp_client_server.cpp
    Environnement.cpp
    VisualAgent.cpp
    ErrorHandler.cpp
    CommunicationManager.cpp
    FipaAclDriver.cpp
    ${BISON_FipaAclParser_OUTPUTS}
    ${FLEX_FipaAclLexer_OUTPUTS}
)

set(HEADERS
    ACLMessage.hpp
    AgentCore.hpp
    AgentID.hpp
    Behaviour.hpp
    Agent.hpp
    AgentService.hpp
    DFAgentDescription.hpp
    udp_client_server.hpp
    Environnement.hpp
    VisualAgent.hpp
    Logger.hpp
    ErrorHandler.hpp
    BehaviourFactory.hpp
    CommunicationManager.hpp
    FipaAclDriver.hpp
)

if(BUILD_GUI)
    list(APPEND SRCS
        gui/EnvironnementGui.cpp
    )
    list(APPEND HEADERS
        gui/EnvironnementGui.hpp
        gui/gAgentGui.hpp
    )
endif()

# ── Bibliothèque ──────────────────────────────────────────────────────────────

set(LIBS_TO_LINK
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    config++
    rt
)

if(BUILD_GUI)
    list(APPEND LIBS_TO_LINK Qt5::Widgets)
endif()

add_library(gagent SHARED ${SRCS} ${HEADERS})

target_link_libraries(gagent ${LIBS_TO_LINK})
