cmake_minimum_required(VERSION 3.16)

project(xWalkCLI VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE)

option(XWALK_CLI_BUILD_HOST "Build the CLI aggregate and deterministic host tests" OFF)
option(XWALK_CLI_BUILD_RPI "Build the CLI aggregate and hardware-labelled executable" OFF)
option(XWALK_CLI_INSTALL_RUNTIME "Install the CLI and deployment resources" OFF)

set(XWALK_INSTALL_CONFIG_DIR "/etc/xwalk" CACHE PATH
    "Administrator-controlled PiCar-X configuration directory")
set(XWALK_INSTALL_STATE_DIR "/var/lib/xwalk" CACHE PATH
    "Writable PiCar-X deployment configuration directory")
set(XWALK_INSTALL_CACHE_DIR "/var/cache/xwalk" CACHE PATH
    "Writable PiCar-X cache directory")
set(XWALK_INSTALL_RUNTIME_DIR "/run/xwalk" CACHE PATH
    "Ephemeral PiCar-X runtime directory")
set(XWALK_INSTALL_DATA_DIR "${CMAKE_INSTALL_DATADIR}/xwalk" CACHE PATH
    "PiCar-X installed resource directory")
set(XWALK_RUNTIME_DATA_DIRECTORY "${CMAKE_INSTALL_FULL_DATADIR}/xwalk" CACHE PATH
    "Absolute PiCar-X packaged-resource directory compiled into applications")

if(XWALK_CLI_BUILD_RPI AND NOT DEFINED XWALK_PICARX_CONFIG_FILE)
    set(XWALK_PICARX_CONFIG_FILE "${XWALK_INSTALL_STATE_DIR}/picar-x.conf"
        CACHE FILEPATH "Writable PiCar-X calibration configuration file")
endif()

if(XWALK_CLI_BUILD_HOST AND XWALK_CLI_BUILD_RPI)
    message(FATAL_ERROR "Host and RPI verification require separate build directories")
endif()

set(XWALK_AGENT_BUILD_HOST ${XWALK_CLI_BUILD_HOST} CACHE BOOL
    "Controlled by XWALK_CLI_BUILD_HOST" FORCE)
set(XWALK_AGENT_BUILD_RPI ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Controlled by XWALK_CLI_BUILD_RPI" FORCE)
set(XWALK_CONTROLLER_BUILD_HOST_TESTS ${XWALK_CLI_BUILD_HOST} CACHE BOOL
    "Controlled by XWALK_CLI_BUILD_HOST" FORCE)
set(XWALK_CONTROLLER_BUILD_RPI_TESTS ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Controlled by XWALK_CLI_BUILD_RPI" FORCE)
set(XWALK_MUSIC_BUILD_ALSA_BACKEND ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Build ALSA music composition for the RPi CLI" FORCE)
set(XWALK_MUSIC_BUILD_SNDFILE_DECODER ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Build native compressed-audio decoding for the RPi CLI" FORCE)
set(XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Build the Ollama provider for the RPi CLI" FORCE)
set(XWALK_CAMERA_BUILD_LINUX_BACKEND ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Build CSI and USB camera capture for the RPi CLI" FORCE)
mark_as_advanced(
    XWALK_AGENT_BUILD_HOST
    XWALK_AGENT_BUILD_RPI
    XWALK_CONTROLLER_BUILD_HOST_TESTS
    XWALK_CONTROLLER_BUILD_RPI_TESTS
    XWALK_MUSIC_BUILD_ALSA_BACKEND
    XWALK_MUSIC_BUILD_SNDFILE_DECODER
    XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER
    XWALK_CAMERA_BUILD_LINUX_BACKEND
)

if(XWALK_CLI_BUILD_HOST OR XWALK_CLI_BUILD_RPI)
    set(BUILD_TESTING ON CACHE BOOL "Enable xWalk CLI aggregate tests" FORCE)
    include(CTest)
endif()

if(NOT TARGET xWalkAgent)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAgent
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkAgent
    )
endif()

add_subdirectory(xWalkController)

add_library(xWalkCLI INTERFACE)
add_library(xWalk::CLI ALIAS xWalkCLI)
target_compile_features(xWalkCLI INTERFACE cxx_std_17)
target_link_libraries(xWalkCLI INTERFACE
    xWalkAgent
    xWalkController
)

if(XWALK_CLI_INSTALL_RUNTIME)
    if(NOT TARGET xwalk-picarx-control)
        message(FATAL_ERROR "CLI installation requires the xwalk-picarx-control executable")
    endif()

    install(TARGETS xwalk-picarx-control
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/xWalkController/config/picar-x.conf"
        DESTINATION "${XWALK_INSTALL_CONFIG_DIR}"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ)
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/xWalkController/config/picar-x-v4.conf"
        "${CMAKE_CURRENT_SOURCE_DIR}/xWalkController/config/picar-x-v5.conf"
        DESTINATION "${XWALK_INSTALL_DATA_DIR}/config")
    install(DIRECTORY DESTINATION "${XWALK_INSTALL_STATE_DIR}")
    install(DIRECTORY DESTINATION "${XWALK_INSTALL_CACHE_DIR}")
    install(DIRECTORY DESTINATION "${XWALK_INSTALL_RUNTIME_DIR}")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkSounds/"
        DESTINATION "${XWALK_INSTALL_DATA_DIR}/sounds"
        FILES_MATCHING PATTERN "*.wav" PATTERN "README.md")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkMusics/"
        DESTINATION "${XWALK_INSTALL_DATA_DIR}/music"
        FILES_MATCHING PATTERN "*.mp3" PATTERN "README.md")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../DevloperNote/Doc/note/Deployment Guide.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../DevloperNote/Doc/note/Release Acceptance Checklist.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../DevloperNote/Doc/note/Host Production Readiness Work.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../DevloperNote/Doc/note/Language Model Provider Configuration.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../DevloperNote/Doc/note/Installation.md"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/xwalk")
    install(PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkTool/shell/setup-rpi.sh"
        "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkTool/shell/provision-hardware.sh"
        DESTINATION "lib/xwalk")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkTool/deployment/udev/99-xwalk-picarx.rules.in"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/xwalk/deployment/udev")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkTool/deployment/systemd/xwalk.service"
        DESTINATION "lib/systemd/system")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkTool/deployment/systemd/xwalk-service.conf"
        DESTINATION "${XWALK_INSTALL_CONFIG_DIR}")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkTool/deployment/tmpfiles/xwalk.conf"
        DESTINATION "lib/tmpfiles.d")
endif()
