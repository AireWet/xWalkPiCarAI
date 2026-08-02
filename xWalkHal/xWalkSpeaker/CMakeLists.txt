cmake_minimum_required(VERSION 3.16)
project(xWalkSpeaker LANGUAGES CXX)

option(XWALK_SPEAKER_BUILD_HOST_TESTS "Build xWalkSpeaker host tests" OFF)
option(XWALK_SPEAKER_BUILD_HARDWARE_TESTS "Build the xWalkSpeaker ALSA hardware test" OFF)
option(XWALK_SPEAKER_BUILD_ALSA_BACKEND "Build the xWalkSpeaker shared ALSA adapter" OFF)

set(XWALK_SPEAKER_NEEDS_ALSA OFF)
if(XWALK_SPEAKER_BUILD_HOST_TESTS OR XWALK_SPEAKER_BUILD_HARDWARE_TESTS OR
    XWALK_SPEAKER_BUILD_ALSA_BACKEND)
    set(XWALK_SPEAKER_NEEDS_ALSA ON)
endif()

if(XWALK_SPEAKER_NEEDS_ALSA AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkSpeaker ALSA adapter requires Linux")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

if(XWALK_SPEAKER_NEEDS_ALSA AND NOT TARGET xWalkAudioAlsa)
    set(XWALK_AUDIO_BUILD_LINUX_BACKEND ON)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAudio ${CMAKE_CURRENT_BINARY_DIR}/xWalkAudio)
endif()

add_library(xWalkSpeaker STATIC
    src/xHal_Rpi5CarSpeakerLifecycle.cpp
    src/xHal_Rpi5CarSpeakerPlayback.cpp
    src/xHal_Rpi5CarSpeakerTasks.cpp
    src/xHal_Rpi5CarSpeakerWorker.cpp
)

target_include_directories(xWalkSpeaker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkSpeaker PUBLIC cxx_std_17)
target_link_libraries(xWalkSpeaker PUBLIC xWalkCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkSpeaker PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_SPEAKER_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkSpeakerTest test/src/xHal_Rpi5CarSpeakerTest.cpp)
    target_link_libraries(xWalkSpeakerTest PRIVATE xWalkSpeaker)
    add_test(NAME xWalkSpeakerHostTest COMMAND xWalkSpeakerTest
        ${CMAKE_CURRENT_BINARY_DIR}/speaker-test.wav
        ${CMAKE_CURRENT_BINARY_DIR}/speaker-test.MP3
        ${CMAKE_CURRENT_BINARY_DIR}/speaker-test.txt
        concurrency)
    set_tests_properties(xWalkSpeakerHostTest PROPERTIES
        LABELS "host"
        TIMEOUT 30)

    if(NOT XWALK_ENABLE_THREAD_SANITIZER)
        add_test(NAME xWalkSpeakerFailureHostTest COMMAND xWalkSpeakerTest
            ${CMAKE_CURRENT_BINARY_DIR}/speaker-failure-test.wav
            ${CMAKE_CURRENT_BINARY_DIR}/speaker-failure-test.MP3
            ${CMAKE_CURRENT_BINARY_DIR}/speaker-failure-test.txt
            failure)
        set_tests_properties(xWalkSpeakerFailureHostTest PROPERTIES
            LABELS "host;failure-isolation"
            TIMEOUT 30)
    endif()
endif()

if(XWALK_SPEAKER_NEEDS_ALSA)
    add_library(xWalkSpeakerAlsa STATIC
        hardware/src/xHal_Rpi5CarSpeakerAlsaCallbacks.cpp
        hardware/src/xHal_Rpi5CarSpeakerAlsaDecode.cpp
        hardware/src/xHal_Rpi5CarSpeakerAlsaLifecycle.cpp
    )
    target_include_directories(xWalkSpeakerAlsa PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkSpeakerAlsa PUBLIC cxx_std_17)
    target_link_libraries(xWalkSpeakerAlsa PUBLIC xWalkSpeaker xWalkAudioAlsa)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkSpeakerAlsa PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()

    if(XWALK_SPEAKER_BUILD_HOST_TESTS)
        add_executable(xWalkSpeakerAlsaTest
            hardware/test/src/xHal_Rpi5CarSpeakerAlsaTest.cpp)
        target_link_libraries(xWalkSpeakerAlsaTest PRIVATE xWalkSpeakerAlsa)
        add_test(NAME xWalkSpeakerAlsaHostTest COMMAND xWalkSpeakerAlsaTest
            ${CMAKE_CURRENT_BINARY_DIR}/speaker-alsa-test.wav
            ${CMAKE_CURRENT_BINARY_DIR}/speaker-alsa-test.ogg
            ${CMAKE_CURRENT_BINARY_DIR}/speaker-alsa-test.mp3)
        set_tests_properties(xWalkSpeakerAlsaHostTest PROPERTIES LABELS "host")
    endif()

    if(XWALK_SPEAKER_BUILD_HARDWARE_TESTS)
        include(CTest)
        add_executable(xWalkSpeakerAlsaHardwareTest
            hardware/test/src/xHal_Rpi5CarSpeakerAlsaHardwareTest.cpp)
        target_link_libraries(xWalkSpeakerAlsaHardwareTest PRIVATE xWalkSpeakerAlsa)
        add_test(NAME xWalkSpeakerAlsaLowVolumeHardwareTest
            COMMAND xWalkSpeakerAlsaHardwareTest)
        set_tests_properties(xWalkSpeakerAlsaLowVolumeHardwareTest PROPERTIES LABELS "hardware")
    endif()
endif()
