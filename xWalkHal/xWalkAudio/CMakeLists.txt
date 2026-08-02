cmake_minimum_required(VERSION 3.16)
project(xWalkAudio LANGUAGES CXX)

option(XWALK_AUDIO_BUILD_HOST_TESTS "Build xWalkAudio ALSA software tests" OFF)
option(XWALK_AUDIO_BUILD_HARDWARE_TESTS "Build xWalkAudio ALSA hardware test" OFF)
option(XWALK_AUDIO_BUILD_LINUX_BACKEND "Build the shared xWalkAudio ALSA backend" OFF)

if((XWALK_AUDIO_BUILD_HOST_TESTS OR XWALK_AUDIO_BUILD_HARDWARE_TESTS OR
    XWALK_AUDIO_BUILD_LINUX_BACKEND) AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkAudio ALSA backend requires Linux")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon
    )
endif()

add_library(xWalkAudio INTERFACE)
target_include_directories(xWalkAudio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAudio INTERFACE cxx_std_17)
target_link_libraries(xWalkAudio INTERFACE xWalkCommon)

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
    target_link_libraries(xWalkAudioAlsa PUBLIC xWalkAudio ALSA::ALSA)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkAudioAlsa PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()

    if(XWALK_AUDIO_BUILD_HOST_TESTS OR XWALK_AUDIO_BUILD_HARDWARE_TESTS)
        include(CTest)
    endif()

    if(XWALK_AUDIO_BUILD_HOST_TESTS)
        add_executable(xWalkAudioAlsaTest
            hardware/test/src/xHal_Rpi5CarAudioAlsaTest.cpp
        )
        target_link_libraries(xWalkAudioAlsaTest PRIVATE xWalkAudioAlsa)
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
