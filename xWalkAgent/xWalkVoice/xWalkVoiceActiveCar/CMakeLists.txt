cmake_minimum_required(VERSION 3.16)
project(xWalkVoiceActiveCar LANGUAGES CXX)

option(XWALK_VOICE_ACTIVE_CAR_BUILD_HOST_TESTS "Build voice-active-car host tests" OFF)

set(XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/config" CACHE PATH
    "Directory containing voice-active-car configuration assets")
if(NOT IS_DIRECTORY "${XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY}")
    message(FATAL_ERROR "Voice-active-car configuration directory does not exist")
endif()
if(NOT EXISTS
    "${XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY}/voice-active-car.jpg")
    message(FATAL_ERROR "Voice-active-car configured image does not exist")
endif()

if(NOT TARGET xWalkSelfDrive)
    set(XWALK_SELF_DRIVE_BUILD_HOST_TESTS OFF CACHE BOOL "Voice car dependency" FORCE)
    set(XWALK_SELF_DRIVE_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Voice car dependency" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkSelfDrive
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSelfDrive)
endif()
if(NOT TARGET xWalkVoiceAssistant)
    set(XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS OFF CACHE BOOL "Voice car dependency" FORCE)
    set(XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Voice car dependency" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/xWalkVoiceAssistant
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceAssistant)
endif()
if(NOT TARGET xWalkLed)
    set(XWALK_LED_BUILD_HOST_TESTS OFF CACHE BOOL "Voice car dependency" FORCE)
    set(XWALK_LED_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Voice car dependency" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/xWalkLed
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLed)
endif()

add_library(xWalkVoiceActiveCar STATIC
    src/xAgent_Rpi5CarVoiceActiveCar.cpp
    src/xAgent_Rpi5CarVoiceActiveCarProfile.cpp)
add_library(xWalk::VoiceActiveCar ALIAS xWalkVoiceActiveCar)
target_include_directories(xWalkVoiceActiveCar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVoiceActiveCar PUBLIC cxx_std_17)
target_link_libraries(xWalkVoiceActiveCar PUBLIC xWalkSelfDrive xWalkVoiceAssistant xWalkLed)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkVoiceActiveCar PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_VOICE_ACTIVE_CAR_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkVoiceActiveCarTest test/src/xAgent_Rpi5CarVoiceActiveCarTest.cpp)
    target_compile_definitions(xWalkVoiceActiveCarTest PRIVATE
        XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY="${XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY}")
    target_link_libraries(xWalkVoiceActiveCarTest PRIVATE xWalkVoiceActiveCar)
    add_test(NAME xWalkVoiceActiveCarHostTest COMMAND xWalkVoiceActiveCarTest)
    set_tests_properties(xWalkVoiceActiveCarHostTest PROPERTIES LABELS "host")
endif()
