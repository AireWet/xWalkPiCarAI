cmake_minimum_required(VERSION 3.16)

project(xWalkController VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLibrary/XWalkDependencies.cmake)

set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE)

option(XWALK_CLI_BUILD_HOST "Build the CLI aggregate and deterministic host tests" OFF)
option(XWALK_CLI_BUILD_RPI "Build the CLI aggregate and hardware-labelled executable" OFF)
option(XWALK_CLI_INSTALL_RUNTIME "Install the CLI and deployment resources" OFF)
option(XWALK_INSTALL_ENCRYPTED_LICENSE
    "Install the provisioned encrypted licence file" OFF)

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

set(XWALK_BUILD_CONFIG_DIR "${CMAKE_BINARY_DIR}/config")
file(MAKE_DIRECTORY "${XWALK_BUILD_CONFIG_DIR}")

set(XWALK_AGENT_HARDWARE_V4_CONFIG_FILE
    "${XWALK_BUILD_CONFIG_DIR}/xwalk-agent-hardware-v4.conf"
    CACHE INTERNAL "Generated Robot HAT v4 hardware-test configuration" FORCE)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/xWalkConfig/xwalk-agent-hardware-v4.conf.in"
    "${XWALK_AGENT_HARDWARE_V4_CONFIG_FILE}"
    @ONLY)

if(XWALK_CLI_BUILD_RPI)
    set(XWALK_PICARX_CONFIG_FILE "${CMAKE_BINARY_DIR}/../runtime/picar-x.conf"
        CACHE FILEPATH "Writable PiCar-X runtime configuration compiled into the build-tree CLI")
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
set(XWALK_SCHEDULER_BUILD_HOST_TESTS ${XWALK_CLI_BUILD_HOST} CACHE BOOL
    "Controlled by XWALK_CLI_BUILD_HOST" FORCE)
set(XWALK_MUSIC_BUILD_ALSA_BACKEND ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Build ALSA music composition for the RPi CLI" FORCE)
set(XWALK_MUSIC_BUILD_SNDFILE_DECODER ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Build native compressed-audio decoding for the RPi CLI" FORCE)
set(XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Build the Ollama provider for the RPi CLI" FORCE)
set(XWALK_CAMERA_BUILD_LINUX_BACKEND ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Build CSI and USB camera capture for the RPi CLI" FORCE)
set(XWALK_CAMERA_BUILD_OPENCV_BACKEND ${XWALK_CLI_BUILD_RPI} CACHE BOOL
    "Build OpenCV encoded camera streaming for the RPi CLI" FORCE)
mark_as_advanced(
    XWALK_AGENT_BUILD_HOST
    XWALK_AGENT_BUILD_RPI
    XWALK_CONTROLLER_BUILD_HOST_TESTS
    XWALK_CONTROLLER_BUILD_RPI_TESTS
    XWALK_SCHEDULER_BUILD_HOST_TESTS
    XWALK_MUSIC_BUILD_ALSA_BACKEND
    XWALK_MUSIC_BUILD_SNDFILE_DECODER
    XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER
    XWALK_CAMERA_BUILD_LINUX_BACKEND
    XWALK_CAMERA_BUILD_OPENCV_BACKEND
)

if(XWALK_CLI_BUILD_HOST OR XWALK_CLI_BUILD_RPI)
    set(BUILD_TESTING ON CACHE BOOL "Enable xWalk CLI aggregate tests" FORCE)
    include(CTest)
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace
    )
endif()

if(NOT TARGET xWalkAgent)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAgent
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkAgent
    )
endif()

add_subdirectory(xWalkScheduler)
add_subdirectory(xWalkApp)

if(XWALK_CLI_BUILD_HOST)
    add_subdirectory(xWalkTest/xSequenceTest)
    add_subdirectory(xWalkTest/xGoogleTest)
endif()

add_library(xWalkControllerAggregate INTERFACE)
add_library(xWalk::CLI ALIAS xWalkControllerAggregate)
target_compile_features(xWalkControllerAggregate INTERFACE cxx_std_17)
target_link_libraries(xWalkControllerAggregate INTERFACE
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
        "${CMAKE_CURRENT_SOURCE_DIR}/xWalkConfig/picar-x.conf"
        DESTINATION "${XWALK_INSTALL_CONFIG_DIR}"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ)
    install(DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/xWalkConfig/picar-x.d"
        DESTINATION "${XWALK_INSTALL_CONFIG_DIR}"
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
    install(FILES
        "${XWALK_LIBRARY_NATIVE_PREFIX}/lib/libvosk.so"
        DESTINATION "lib/xwalk")
    install(DIRECTORY
        "${XWALK_LIBRARY_COMMON_PREFIX}/models/vosk/vosk-model-small-en-us-0.15"
        DESTINATION "${XWALK_INSTALL_DATA_DIR}/models/vosk")
    install(FILES
        "${XWALK_LIBRARY_COMMON_PREFIX}/models/vosk/LICENSE"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/xwalk/vosk")
    install(DIRECTORY DESTINATION "${XWALK_INSTALL_STATE_DIR}")
    install(DIRECTORY DESTINATION "${XWALK_INSTALL_CACHE_DIR}")
    install(DIRECTORY DESTINATION "${XWALK_INSTALL_RUNTIME_DIR}")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAudioResources/sounds/"
        DESTINATION "${XWALK_INSTALL_DATA_DIR}/sounds"
        FILES_MATCHING PATTERN "*.wav")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAudioResources/music/"
        DESTINATION "${XWALK_INSTALL_DATA_DIR}/music"
        FILES_MATCHING PATTERN "*.mp3")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAudioResources/README.md"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/xwalk/audio-resources")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../devloper-note/xwalk-rpi5-note/Doc/note/Deployment Guide.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../devloper-note/xwalk-rpi5-note/Doc/note/Release Acceptance Checklist.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../devloper-note/xwalk-rpi5-note/Doc/note/Host Production Readiness Work.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../devloper-note/xwalk-rpi5-note/Doc/note/Language Model Provider Configuration.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../devloper-note/xwalk-rpi5-note/Doc/note/License Key Workflow.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../devloper-note/xwalk-rpi5-note/Doc/note/Installation.md"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/xwalk")
    install(PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/shell-agent/deploy-tool/setup-rpi.sh"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/shell-agent/deploy-tool/provision-hardware.sh"
        DESTINATION "lib/xwalk")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/shell-agent/deploy-tool/rpi-defaults.conf"
        DESTINATION "lib/xwalk")
    install(PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh"
        DESTINATION "lib/xwalk/xWalkTool/shell-agent/env-tool/license")
    install(PROGRAMS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/py-agent/dev-tool/xWalkLicenseTool"
        DESTINATION "lib/xwalk/xWalkTool/py-agent/dev-tool")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/shell-agent/env-tool/license/xWalkLicense.cfg"
        DESTINATION "lib/xwalk/xWalkTool/shell-agent/env-tool/license")
    if(XWALK_INSTALL_ENCRYPTED_LICENSE)
        set(xwalk_encrypted_license
            "${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLibrary/X_WALK_LICENSE.KEY")
        if(NOT EXISTS "${xwalk_encrypted_license}")
            message(FATAL_ERROR
                "XWALK_INSTALL_ENCRYPTED_LICENSE requires ${xwalk_encrypted_license}")
        endif()
        file(SIZE "${xwalk_encrypted_license}" xwalk_encrypted_license_size)
        file(READ "${xwalk_encrypted_license}" xwalk_encrypted_license_header
            OFFSET 0 LIMIT 4 HEX)
        if(xwalk_encrypted_license_size LESS_EQUAL 44 OR
            NOT xwalk_encrypted_license_header STREQUAL "58574c31")
            message(FATAL_ERROR
                "X_WALK_LICENSE.KEY is not a provisioned XWL1 encrypted file")
        endif()
        install(FILES "${xwalk_encrypted_license}"
            DESTINATION "lib/xwalk/xWalkLibrary"
            PERMISSIONS OWNER_READ OWNER_WRITE)
    endif()
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/shell-agent/deploy-tool/udev/99-xwalk-picarx.rules.in"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/xwalk/deployment/udev")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/shell-agent/deploy-tool/systemd/xwalk.service"
        DESTINATION "lib/systemd/system")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/shell-agent/deploy-tool/systemd/xwalk-service.conf"
        DESTINATION "${XWALK_INSTALL_CONFIG_DIR}"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ)
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTool/shell-agent/deploy-tool/tmpfiles/xwalk.conf"
        DESTINATION "lib/tmpfiles.d")
endif()
