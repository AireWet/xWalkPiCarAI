cmake_minimum_required(VERSION 3.16)
project(xWalkMusic LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/XWalkDependencies.cmake)

option(XWALK_MUSIC_BUILD_HOST_TESTS "Build xWalkMusic host tests" OFF)
option(XWALK_MUSIC_BUILD_HARDWARE_TESTS "Build the xWalkMusic ALSA hardware test" OFF)
option(XWALK_MUSIC_BUILD_ALSA_BACKEND "Build the xWalkMusic shared ALSA adapter" OFF)
option(XWALK_MUSIC_BUILD_SNDFILE_DECODER "Build native libsndfile audio decoding" OFF)

set(XWALK_MUSIC_NEEDS_ALSA OFF)
if(XWALK_MUSIC_BUILD_HOST_TESTS OR XWALK_MUSIC_BUILD_HARDWARE_TESTS OR
    XWALK_MUSIC_BUILD_ALSA_BACKEND)
    set(XWALK_MUSIC_NEEDS_ALSA ON)
endif()

if(XWALK_MUSIC_NEEDS_ALSA AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkMusic ALSA adapter requires Linux")
endif()

if(XWALK_MUSIC_BUILD_SNDFILE_DECODER AND NOT XWALK_MUSIC_NEEDS_ALSA)
    message(FATAL_ERROR "xWalkMusic libsndfile decoder requires the ALSA adapter")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

if(XWALK_MUSIC_NEEDS_ALSA AND NOT TARGET xWalkAudioAlsa)
    set(XWALK_AUDIO_BUILD_LINUX_BACKEND ON)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkAudio ${CMAKE_CURRENT_BINARY_DIR}/xWalkAudio)
endif()

add_library(xWalkMusic STATIC
    src/xHal_Rpi5CarMusicLifecycle.cpp
    src/xHal_Rpi5CarMusicPlayback.cpp
    src/xHal_Rpi5CarMusicTheory.cpp
    src/xHal_Rpi5CarMusicTone.cpp
)

target_include_directories(xWalkMusic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkMusic PUBLIC cxx_std_17)
target_link_libraries(xWalkMusic PUBLIC xWalkLibraryCommon PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkMusic PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_MUSIC_BUILD_HOST_TESTS)
    include(CTest)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_MUSIC_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_MUSIC_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarMusicTraceConfig.py)
    set(XWALK_MUSIC_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkMusicTrace.xml)
    add_custom_command(OUTPUT ${XWALK_MUSIC_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_MUSIC_TRACE_GENERATOR}
            --input ${XWALK_MUSIC_TRACE_INVENTORY}
            --output ${XWALK_MUSIC_TRACE_CONFIG}
        DEPENDS ${XWALK_MUSIC_TRACE_INVENTORY} ${XWALK_MUSIC_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkMusic trace catalogue" VERBATIM)
    add_custom_target(xWalkMusicTestTraceConfig ALL DEPENDS ${XWALK_MUSIC_TRACE_CONFIG})
    add_dependencies(xWalkMusicTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkMusicTest
        test/src/xHal_Rpi5CarMusicTest.cpp
        test/src/xHal_Rpi5CarMusicTestSupport.cpp
        simulation/src/xHal_Rpi5CarMusicSimulationArguments.cpp)
    target_include_directories(xWalkMusicTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_MUSIC_TEST_TRACE_LOG
        ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkMusicTest.log)
    target_compile_definitions(xWalkMusicTest PRIVATE
        XWALK_MUSIC_SIMULATION_TRACE_CONFIG_PATH="${XWALK_MUSIC_TRACE_CONFIG}"
        XWALK_MUSIC_SIMULATION_TRACE_LOG_PATH="${XWALK_MUSIC_TEST_TRACE_LOG}")
    target_link_libraries(xWalkMusicTest PRIVATE xWalkMusic xWalkTrace)
    add_dependencies(xWalkMusicTest xWalkMusicTestTraceConfig)
    add_test(NAME xWalkMusicHostTest COMMAND xWalkMusicTest)
    set_tests_properties(xWalkMusicHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_MUSIC_NEEDS_ALSA)
    add_library(xWalkMusicAlsa STATIC
        hardware/src/xHal_Rpi5CarMusicAlsaCallbacks.cpp
        hardware/src/xHal_Rpi5CarMusicAlsaDecode.cpp
        hardware/src/xHal_Rpi5CarMusicAlsaLifecycle.cpp
        hardware/src/xHal_Rpi5CarMusicAlsaPlayback.cpp
    )
target_link_libraries(xWalkMusicAlsa PRIVATE xWalkTrace)
    target_include_directories(xWalkMusicAlsa PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkMusicAlsa PUBLIC cxx_std_17)
    target_link_libraries(xWalkMusicAlsa PUBLIC xWalkMusic xWalkAudioAlsa)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkMusicAlsa PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()

    if(XWALK_MUSIC_BUILD_SNDFILE_DECODER)
        find_path(XWALK_SNDFILE_INCLUDE_DIRECTORY NAMES sndfile.h)
        find_library(XWALK_SNDFILE_LIBRARY NAMES sndfile)
        if(NOT XWALK_SNDFILE_INCLUDE_DIRECTORY OR NOT XWALK_SNDFILE_LIBRARY)
            message(FATAL_ERROR
                "xWalkMusic native audio decoding requires libsndfile development files")
        endif()
        mark_as_advanced(XWALK_SNDFILE_INCLUDE_DIRECTORY XWALK_SNDFILE_LIBRARY)
        add_library(xWalkSndFileDependency UNKNOWN IMPORTED)
        set_target_properties(xWalkSndFileDependency PROPERTIES
            IMPORTED_LOCATION "${XWALK_SNDFILE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${XWALK_SNDFILE_INCLUDE_DIRECTORY}")
        add_library(xWalkMusicSndFileDecoder STATIC
            hardware/src/xHal_Rpi5CarMusicSndFileDecoder.cpp)
target_link_libraries(xWalkMusicSndFileDecoder PRIVATE xWalkTrace)
        target_include_directories(xWalkMusicSndFileDecoder PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
        target_compile_features(xWalkMusicSndFileDecoder PUBLIC cxx_std_17)
        target_link_libraries(xWalkMusicSndFileDecoder
            PUBLIC xWalkMusicAlsa
            PRIVATE xWalkSndFileDependency)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(xWalkMusicSndFileDecoder PRIVATE
                -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
        endif()

        if(XWALK_MUSIC_BUILD_HOST_TESTS)
            add_executable(xWalkMusicSndFileDecoderTest
                hardware/test/src/xHal_Rpi5CarMusicSndFileDecoderTest.cpp)
            target_link_libraries(xWalkMusicSndFileDecoderTest PRIVATE
                xWalkMusicSndFileDecoder)
            add_test(NAME xWalkMusicSndFileDecoderHostTest
                COMMAND xWalkMusicSndFileDecoderTest
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkAudioResources/music/slow-trail-Ahjay_Stelino.mp3)
            set_tests_properties(xWalkMusicSndFileDecoderHostTest PROPERTIES LABELS "host")
        endif()
    endif()

    if(XWALK_MUSIC_BUILD_HOST_TESTS)
        add_executable(xWalkMusicAlsaTest
            hardware/test/src/xHal_Rpi5CarMusicAlsaTest.cpp)
        target_include_directories(xWalkMusicAlsaTest PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/hardware/test/include)
        target_link_libraries(xWalkMusicAlsaTest PRIVATE xWalkMusicAlsa)
        add_test(NAME xWalkMusicAlsaHostTest COMMAND xWalkMusicAlsaTest
            ${CMAKE_CURRENT_BINARY_DIR}/music-alsa-test.wav)
        set_tests_properties(xWalkMusicAlsaHostTest PROPERTIES LABELS "host")
    endif()

    if(XWALK_MUSIC_BUILD_HARDWARE_TESTS)
        include(CTest)
        add_executable(xWalkMusicAlsaHardwareTest
            hardware/test/src/xHal_Rpi5CarMusicAlsaHardwareTest.cpp)
target_link_libraries(xWalkMusicAlsaHardwareTest PRIVATE xWalkTrace)
        target_link_libraries(xWalkMusicAlsaHardwareTest PRIVATE xWalkMusicAlsa)
        add_test(NAME xWalkMusicAlsaLowVolumeHardwareTest COMMAND xWalkMusicAlsaHardwareTest)
        set_tests_properties(xWalkMusicAlsaLowVolumeHardwareTest PROPERTIES LABELS "hardware")
    endif()
endif()
