cmake_minimum_required(VERSION 3.16)
project(xWalkAudio LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/XWalkDependencies.cmake)

option(XWALK_AUDIO_BUILD_HOST_TESTS "Build xWalkAudio ALSA software tests" OFF)
option(XWALK_AUDIO_BUILD_HARDWARE_TESTS "Build xWalkAudio ALSA hardware test" OFF)
option(XWALK_AUDIO_BUILD_LINUX_BACKEND "Build the shared xWalkAudio ALSA backend" OFF)

if((XWALK_AUDIO_BUILD_HOST_TESTS OR XWALK_AUDIO_BUILD_HARDWARE_TESTS OR
    XWALK_AUDIO_BUILD_LINUX_BACKEND) AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkAudio ALSA backend requires Linux")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common
    )
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkAudio INTERFACE)
target_include_directories(xWalkAudio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAudio INTERFACE cxx_std_17)
target_link_libraries(xWalkAudio INTERFACE xWalkLibraryCommon)

if(XWALK_AUDIO_BUILD_HOST_TESTS OR XWALK_AUDIO_BUILD_HARDWARE_TESTS OR
    XWALK_AUDIO_BUILD_LINUX_BACKEND)
    find_package(ALSA REQUIRED)

    add_library(xWalkAudioAlsa STATIC
        hardware/src/xHal_Rpi5CarAudioAlsa.cpp
        hardware/src/xHal_Rpi5CarAudioAlsaLifecycle.cpp
        hardware/src/xHal_Rpi5CarAudioAlsaSystem.cpp
    )
    target_include_directories(xWalkAudioAlsa PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include
    )
    target_compile_features(xWalkAudioAlsa PUBLIC cxx_std_17)
    target_link_libraries(xWalkAudioAlsa
        PUBLIC xWalkAudio ALSA::ALSA
        PRIVATE xWalkTrace)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkAudioAlsa PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()

    if(XWALK_AUDIO_BUILD_HOST_TESTS OR XWALK_AUDIO_BUILD_HARDWARE_TESTS)
        include(CTest)
    endif()

    if(XWALK_AUDIO_BUILD_HOST_TESTS)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        set(XWALK_AUDIO_TEST_TRACE_INVENTORY
            ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
        set(XWALK_AUDIO_TEST_TRACE_GENERATOR
            ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarAudioTraceConfig.py)
        set(XWALK_AUDIO_TEST_TRACE_CONFIG
            ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkAudioTrace.xml)
        add_custom_command(
            OUTPUT ${XWALK_AUDIO_TEST_TRACE_CONFIG}
            COMMAND ${Python3_EXECUTABLE} ${XWALK_AUDIO_TEST_TRACE_GENERATOR}
                --input ${XWALK_AUDIO_TEST_TRACE_INVENTORY}
                --output ${XWALK_AUDIO_TEST_TRACE_CONFIG}
            DEPENDS ${XWALK_AUDIO_TEST_TRACE_INVENTORY}
                ${XWALK_AUDIO_TEST_TRACE_GENERATOR}
            COMMENT "Preparing the persistent xWalkAudio host-test catalogue"
            VERBATIM)
        add_custom_target(xWalkAudioTestTraceConfig
            DEPENDS ${XWALK_AUDIO_TEST_TRACE_CONFIG})
        add_dependencies(xWalkAudioTestTraceConfig xWalkTraceMetadata)

        add_executable(xWalkAudioAlsaTest
            hardware/test/src/xHal_Rpi5CarAudioAlsaTest.cpp
            hardware/test/src/xHal_Rpi5CarAudioAlsaTestSupport.cpp
            simulation/src/xHal_Rpi5CarAudioHandler.cpp
            simulation/src/xHal_Rpi5CarAudioHostStub.cpp
            simulation/src/xHal_Rpi5CarAudioSimulationArguments.cpp
        )
        target_include_directories(xWalkAudioAlsaTest PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/hardware/test/include
            ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
        target_compile_definitions(xWalkAudioAlsaTest PRIVATE
            XWALK_AUDIO_SIMULATION_TRACE_CONFIG_PATH="${XWALK_AUDIO_TEST_TRACE_CONFIG}"
            XWALK_AUDIO_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkAudioTest.log")
        target_link_libraries(xWalkAudioAlsaTest PRIVATE xWalkAudioAlsa xWalkTrace)
        add_dependencies(xWalkAudioAlsaTest xWalkAudioTestTraceConfig)
        add_test(NAME xWalkAudioAlsaSoftwareTest COMMAND xWalkAudioAlsaTest)
        set_tests_properties(xWalkAudioAlsaSoftwareTest PROPERTIES LABELS "host")
    endif()

    if(XWALK_AUDIO_BUILD_HARDWARE_TESTS)
        add_executable(xWalkAudioAlsaHardwareTest
            hardware/test/src/xHal_Rpi5CarAudioAlsaHardwareTest.cpp
        )
        target_link_libraries(xWalkAudioAlsaHardwareTest PRIVATE xWalkAudioAlsa)
        add_test(NAME xWalkAudioAlsaSilentHardwareTest COMMAND xWalkAudioAlsaHardwareTest)
        set_tests_properties(xWalkAudioAlsaSilentHardwareTest PROPERTIES LABELS "hardware")
    endif()
endif()
