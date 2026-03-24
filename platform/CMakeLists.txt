find_package(Boost COMPONENTS filesystem program_options REQUIRED)

add_executable(gAgentPlatform
    main.cpp
    common/Args.cpp
    ams/AMS.cpp
    df/DF.cpp
    monitor/Monitor.cpp
    manager/Manager.cpp
)

target_include_directories(gAgentPlatform PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(gAgentPlatform
    gagent
    Boost::filesystem
    Boost::program_options
    config++
)

# Symlinks dans le répertoire de build
add_custom_command(TARGET gAgentPlatform POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:gAgentPlatform>
        $<TARGET_FILE_DIR:gAgentPlatform>/agentplatform
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:gAgentPlatform>
        $<TARGET_FILE_DIR:gAgentPlatform>/agentmanager
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:gAgentPlatform>
        $<TARGET_FILE_DIR:gAgentPlatform>/agentmonitor
    COMMENT "Création des symlinks agentplatform / agentmanager / agentmonitor"
)

# Installation
install(TARGETS gAgentPlatform RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(CODE "
    file(CREATE_LINK gAgentPlatform
        \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/agentplatform\" SYMBOLIC)
    file(CREATE_LINK gAgentPlatform
        \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/agentmanager\"  SYMBOLIC)
    file(CREATE_LINK gAgentPlatform
        \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/agentmonitor\"  SYMBOLIC)
")
