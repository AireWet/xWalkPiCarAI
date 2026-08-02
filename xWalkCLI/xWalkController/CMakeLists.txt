cmake_minimum_required(VERSION 3.16)

project(xWalkController LANGUAGES CXX)

option(XWALK_CONTROLLER_BUILD_HOST_TESTS "Build PiCar-X CLI host tests" OFF)
option(XWALK_CONTROLLER_BUILD_RPI_TESTS "Build PiCar-X CLI RPi application and test" OFF)

set(XWALK_CONTROLLER_HELP_JSON "${CMAKE_CURRENT_SOURCE_DIR}/config/help.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${XWALK_CONTROLLER_HELP_JSON}")
file(STRINGS "${XWALK_CONTROLLER_HELP_JSON}" XWALK_CONTROLLER_HELP_LINES
    REGEX "^        \".*\"[,]?$")
list(LENGTH XWALK_CONTROLLER_HELP_LINES XWALK_CONTROLLER_HELP_LINE_COUNT)
if(XWALK_CONTROLLER_HELP_LINE_COUNT EQUAL 0)
    message(FATAL_ERROR "PiCar-X CLI help configuration contains no help lines")
endif()

set(XWALK_CONTROLLER_GENERATED_INCLUDE_DIRECTORY
    "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
set(XWALK_CONTROLLER_GENERATED_HELP_HEADER
    "${XWALK_CONTROLLER_GENERATED_INCLUDE_DIRECTORY}/xAgent_Rpi5CarControllerHelpGenerated.h")
file(MAKE_DIRECTORY "${XWALK_CONTROLLER_GENERATED_INCLUDE_DIRECTORY}")
file(WRITE "${XWALK_CONTROLLER_GENERATED_HELP_HEADER}"
    "#ifndef XAGENT_RPI5CAR_CONTROLLER_HELP_GENERATED_H\n"
    "#define XAGENT_RPI5CAR_CONTROLLER_HELP_GENERATED_H\n\n"
    "namespace xwalk::agent\n{\n\n"
    "inline constexpr hal::cstring XAGENT_RPI5CAR_CONTROLLER_HELP =\n")
math(EXPR XWALK_CONTROLLER_HELP_LAST_INDEX
    "${XWALK_CONTROLLER_HELP_LINE_COUNT} - 1")
foreach(XWALK_CONTROLLER_HELP_INDEX
    RANGE 0 ${XWALK_CONTROLLER_HELP_LAST_INDEX})
    list(GET XWALK_CONTROLLER_HELP_LINES
        ${XWALK_CONTROLLER_HELP_INDEX} XWALK_CONTROLLER_HELP_LINE)
    string(STRIP "${XWALK_CONTROLLER_HELP_LINE}"
        XWALK_CONTROLLER_HELP_LINE)
    string(REGEX REPLACE ",$" "" XWALK_CONTROLLER_HELP_LINE
        "${XWALK_CONTROLLER_HELP_LINE}")
    if(XWALK_CONTROLLER_HELP_INDEX LESS XWALK_CONTROLLER_HELP_LAST_INDEX)
        file(APPEND "${XWALK_CONTROLLER_GENERATED_HELP_HEADER}"
            "    ${XWALK_CONTROLLER_HELP_LINE} \"\\n\"\n")
    else()
        file(APPEND "${XWALK_CONTROLLER_GENERATED_HELP_HEADER}"
            "    ${XWALK_CONTROLLER_HELP_LINE};\n")
    endif()
endforeach()
file(APPEND "${XWALK_CONTROLLER_GENERATED_HELP_HEADER}"
    "\n} /* namespace xwalk::agent */\n\n"
    "#endif /* XAGENT_RPI5CAR_CONTROLLER_HELP_GENERATED_H */\n")

if(XWALK_CONTROLLER_BUILD_RPI_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkController hardware mode requires Linux I2C and GPIO support")
endif()

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL "CLI dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS ${XWALK_CONTROLLER_BUILD_RPI_TESTS}
        CACHE BOOL "CLI hardware dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkLineTracking)
    set(XWALK_LINE_TRACKING_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    set(XWALK_LINE_TRACKING_BUILD_HARDWARE_TESTS ${XWALK_CONTROLLER_BUILD_RPI_TESTS}
        CACHE BOOL "CLI hardware dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkLineTracking
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLineTracking)
endif()

if(NOT TARGET xWalkSelfDrive)
    set(XWALK_MUSIC_BUILD_ALSA_BACKEND ${XWALK_CONTROLLER_BUILD_RPI_TESTS} CACHE BOOL
        "CLI RPi audio dependency setting" FORCE)
    set(XWALK_MUSIC_BUILD_SNDFILE_DECODER ${XWALK_CONTROLLER_BUILD_RPI_TESTS} CACHE BOOL
        "CLI RPi compressed-audio dependency setting" FORCE)
    set(XWALK_SELF_DRIVE_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    set(XWALK_SELF_DRIVE_BUILD_HARDWARE_TESTS ${XWALK_CONTROLLER_BUILD_RPI_TESTS}
        CACHE BOOL "CLI hardware dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkSelfDrive
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSelfDrive)
endif()

if(NOT TARGET xWalkBoot)
    set(XWALK_BOOT_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI boot dependency setting" FORCE)
    set(XWALK_BOOT_BUILD_RPI_BACKEND ${XWALK_CONTROLLER_BUILD_RPI_TESTS} CACHE BOOL
        "CLI RPi boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkBoot
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkBoot)
endif()

add_library(xWalkController STATIC
    src/xAgent_Rpi5CarControllerCommands.cpp
    src/xAgent_Rpi5CarControllerLifecycle.cpp
    src/xAgent_Rpi5CarControllerParsing.cpp
)
add_library(xWalk::Controller ALIAS xWalkController)

target_include_directories(xWalkController
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${XWALK_CONTROLLER_GENERATED_INCLUDE_DIRECTORY})
target_compile_definitions(xWalkController PRIVATE
    XWALK_CONTROLLER_USE_GENERATED_HELP=1)
target_compile_features(xWalkController PUBLIC cxx_std_17)
target_link_libraries(xWalkController PUBLIC
    xWalkPicarx
    xWalkLineTracking
    xWalkLocalVoiceChatbot
    xWalkVoiceActiveCar
    xWalkSelfDrive)
target_link_libraries(xWalkController PUBLIC xWalkSpiTransfer)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkController PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_CONTROLLER_BUILD_HOST_TESTS OR XWALK_CONTROLLER_BUILD_RPI_TESTS)
    include(CTest)
endif()

if(XWALK_CONTROLLER_BUILD_HOST_TESTS)
    add_executable(xwalk-picarx-control app/xAgent_Rpi5CarControllerHostMain.cpp)
    target_compile_definitions(xwalk-picarx-control PRIVATE
        XWALK_RUNTIME_DATA_DIRECTORY="${XWALK_RUNTIME_DATA_DIRECTORY}")
    target_link_libraries(xwalk-picarx-control PRIVATE xWalkController)
    add_executable(xWalkControllerTest test/src/xAgent_Rpi5CarControllerTest.cpp)
    target_compile_definitions(xWalkControllerTest PRIVATE
        XWALK_TEST_SOUND_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkSounds")
    target_link_libraries(xWalkControllerTest PRIVATE xWalkController)
    add_test(NAME xWalkControllerHostTest COMMAND xWalkControllerTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/cli-test.conf)
    set_tests_properties(xWalkControllerHostTest PROPERTIES LABELS "host")
    add_test(NAME xWalkControllerHostCliHelpTest COMMAND xwalk-picarx-control --help)
    set_tests_properties(xWalkControllerHostCliHelpTest PROPERTIES
        LABELS "host;deployment"
        WORKING_DIRECTORY "/tmp")
    add_test(NAME xWalkControllerHostCliInvalidConfigTest
        COMMAND xwalk-picarx-control --deployment-config relative.conf --help)
    set_tests_properties(xWalkControllerHostCliInvalidConfigTest PROPERTIES
        LABELS "host;deployment"
        WILL_FAIL TRUE)
endif()

if(XWALK_CONTROLLER_BUILD_RPI_TESTS)
    if(NOT TARGET xWalkBootRpi)
        message(FATAL_ERROR "xWalkController RPi application requires xWalkBootRpi")
    endif()
    add_executable(xwalk-picarx-control app/xAgent_Rpi5CarControllerMain.cpp)
    target_compile_definitions(xwalk-picarx-control PRIVATE
        XWALK_PICARX_CONFIG_FILE="${XWALK_PICARX_CONFIG_FILE}"
        XWALK_RUNTIME_DATA_DIRECTORY="${XWALK_RUNTIME_DATA_DIRECTORY}")
    target_link_libraries(xwalk-picarx-control PRIVATE
        xWalkController
        xWalkBootRpi
    )
    add_test(NAME xWalkControllerHardwareHelpTest COMMAND xwalk-picarx-control --help)
    set_tests_properties(xWalkControllerHardwareHelpTest PROPERTIES LABELS "hardware")
endif()
