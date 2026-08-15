cmake_minimum_required(VERSION 3.16)
project(xWalkGPT LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/XWalkDependencies.cmake)

option(XWALK_GPT_BUILD_HOST_TESTS "Build xWalkGPT host tests" OFF)
option(XWALK_GPT_BUILD_HARDWARE_TESTS "Build xWalkGPT target tests" OFF)
option(XWALK_GPT_BUILD_STT_ALSA_BACKEND "Build the ALSA speech-to-text backend" OFF)
option(XWALK_GPT_BUILD_TTS_ALSA_BACKEND "Build the ALSA text-to-speech backend" OFF)
option(XWALK_GPT_BUILD_VOSK_PROVIDER "Build the runtime-loaded Vosk provider" OFF)
option(XWALK_GPT_BUILD_ESPEAK_PROVIDER "Build the Espeak process provider" OFF)
option(XWALK_GPT_BUILD_PIPER_PROVIDER "Build the Piper process provider" OFF)
option(XWALK_GPT_BUILD_PICO2WAVE_PROVIDER "Build the Pico2Wave process provider" OFF)

set(XWALK_GPT_NEEDS_STT_ALSA OFF)
if(XWALK_GPT_BUILD_HOST_TESTS OR XWALK_GPT_BUILD_HARDWARE_TESTS OR
    XWALK_GPT_BUILD_STT_ALSA_BACKEND OR XWALK_GPT_BUILD_VOSK_PROVIDER)
    set(XWALK_GPT_NEEDS_STT_ALSA ON)
endif()

set(XWALK_GPT_NEEDS_TTS_ALSA OFF)
if(XWALK_GPT_BUILD_HOST_TESTS OR XWALK_GPT_BUILD_HARDWARE_TESTS OR
    XWALK_GPT_BUILD_TTS_ALSA_BACKEND OR XWALK_GPT_BUILD_ESPEAK_PROVIDER)
    set(XWALK_GPT_NEEDS_TTS_ALSA ON)
endif()

if((XWALK_GPT_BUILD_VOSK_PROVIDER OR XWALK_GPT_BUILD_ESPEAK_PROVIDER OR
    XWALK_GPT_BUILD_PIPER_PROVIDER OR XWALK_GPT_BUILD_PICO2WAVE_PROVIDER) AND
    NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkGPT offline voice providers require Linux")
endif()

if((XWALK_GPT_NEEDS_STT_ALSA OR XWALK_GPT_NEEDS_TTS_ALSA) AND
    NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkGPT ALSA speech backends require Linux")
endif()

set(XWALK_BOARD_CONTROL_BUILD_HOST_TESTS ${XWALK_GPT_BUILD_HOST_TESTS})

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

if(NOT TARGET xWalkBoardControl)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkBoardControl
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkBoardControl)
endif()

if(XWALK_GPT_NEEDS_TTS_ALSA AND NOT TARGET xWalkAudio)
    set(XWALK_AUDIO_BUILD_LINUX_BACKEND ON)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkAudio
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkAudio)
endif()

if(XWALK_GPT_NEEDS_TTS_ALSA AND NOT TARGET xWalkAudioAlsa)
    message(FATAL_ERROR "xWalkGPT text-to-speech requires the xWalkAudio ALSA backend")
endif()

add_library(xWalkGPT STATIC
    src/xHal_Rpi5CarSpeechToText.cpp
    src/xHal_Rpi5CarSpeechToTextLifecycle.cpp
    src/xHal_Rpi5CarTextToSpeech.cpp
    src/xHal_Rpi5CarTextToSpeechLifecycle.cpp
)

target_include_directories(xWalkGPT PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkGPT PUBLIC cxx_std_17)
target_link_libraries(xWalkGPT PUBLIC xWalkBoardControl xWalkLibraryCommon PRIVATE xWalkTrace)

if(XWALK_GPT_NEEDS_STT_ALSA)
    find_package(ALSA REQUIRED)
    add_library(xWalkSpeechToTextAlsa STATIC
        hardware/src/xHal_Rpi5CarSpeechToTextAlsa.cpp
        hardware/src/xHal_Rpi5CarSpeechToTextAlsaSystem.cpp)
target_link_libraries(xWalkSpeechToTextAlsa PRIVATE xWalkTrace)
    target_include_directories(xWalkSpeechToTextAlsa PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkSpeechToTextAlsa PUBLIC cxx_std_17)
    target_link_libraries(xWalkSpeechToTextAlsa PUBLIC xWalkGPT ALSA::ALSA)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkSpeechToTextAlsa PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(XWALK_GPT_NEEDS_TTS_ALSA)
    add_library(xWalkTextToSpeechAlsa STATIC
        hardware/src/xHal_Rpi5CarTextToSpeechAlsa.cpp)
target_link_libraries(xWalkTextToSpeechAlsa PRIVATE xWalkTrace)
    target_include_directories(xWalkTextToSpeechAlsa PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkTextToSpeechAlsa PUBLIC cxx_std_17)
    target_link_libraries(xWalkTextToSpeechAlsa PUBLIC xWalkGPT xWalkAudioAlsa)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkTextToSpeechAlsa PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(XWALK_GPT_BUILD_VOSK_PROVIDER)
    add_library(xWalkSpeechRecognizerVosk STATIC
        hardware/src/xHal_Rpi5CarSpeechRecognizerVosk.cpp
        hardware/src/xHal_Rpi5CarVoskRecognizerGuard.cpp)
target_link_libraries(xWalkSpeechRecognizerVosk PRIVATE xWalkTrace)
    target_include_directories(xWalkSpeechRecognizerVosk PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkSpeechRecognizerVosk PUBLIC cxx_std_17)
    target_link_libraries(xWalkSpeechRecognizerVosk PUBLIC
        xWalkSpeechToTextAlsa ${CMAKE_DL_LIBS})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkSpeechRecognizerVosk PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(XWALK_GPT_BUILD_ESPEAK_PROVIDER)
    add_library(xWalkTextToSpeechEspeak STATIC
        hardware/src/xHal_Rpi5CarTextToSpeechEspeak.cpp)
target_link_libraries(xWalkTextToSpeechEspeak PRIVATE xWalkTrace)
    target_include_directories(xWalkTextToSpeechEspeak PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkTextToSpeechEspeak PUBLIC cxx_std_17)
    target_link_libraries(xWalkTextToSpeechEspeak PUBLIC xWalkTextToSpeechAlsa)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkTextToSpeechEspeak PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(XWALK_GPT_BUILD_PIPER_PROVIDER)
    add_library(xWalkTextToSpeechPiper STATIC
        hardware/src/xHal_Rpi5CarTextToSpeechPiper.cpp)
target_link_libraries(xWalkTextToSpeechPiper PRIVATE xWalkTrace)
    target_include_directories(xWalkTextToSpeechPiper PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkTextToSpeechPiper PUBLIC cxx_std_17)
    target_link_libraries(xWalkTextToSpeechPiper PUBLIC xWalkGPT)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkTextToSpeechPiper PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(XWALK_GPT_BUILD_PICO2WAVE_PROVIDER)
    add_library(xWalkTextToSpeechPico2Wave STATIC
        hardware/src/xHal_Rpi5CarTextToSpeechPico2Wave.cpp)
target_link_libraries(xWalkTextToSpeechPico2Wave PRIVATE xWalkTrace)
    target_include_directories(xWalkTextToSpeechPico2Wave PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkTextToSpeechPico2Wave PUBLIC cxx_std_17)
    target_link_libraries(xWalkTextToSpeechPico2Wave PUBLIC xWalkGPT)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkTextToSpeechPico2Wave PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkGPT PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_GPT_BUILD_HOST_TESTS OR XWALK_GPT_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_GPT_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_GPT_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_GPT_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarGptTraceConfig.py)
    set(XWALK_GPT_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkGptTrace.xml)
    add_custom_command(OUTPUT ${XWALK_GPT_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_GPT_TRACE_GENERATOR}
            --input ${XWALK_GPT_TRACE_INVENTORY} --output ${XWALK_GPT_TRACE_CONFIG}
        DEPENDS ${XWALK_GPT_TRACE_INVENTORY} ${XWALK_GPT_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkGPT trace catalogue" VERBATIM)
    add_custom_target(xWalkGptTestTraceConfig ALL DEPENDS ${XWALK_GPT_TRACE_CONFIG})
    add_dependencies(xWalkGptTestTraceConfig xWalkTraceMetadata)
    set(XWALK_GPT_TEST_DEFINITIONS
        XWALK_GPT_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GPT_TRACE_CONFIG}"
        XWALK_GPT_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkGptTest.log")
    add_executable(xWalkSpeechToTextTest
        test/src/xHal_Rpi5CarSpeechToTextTest.cpp
        test/src/xHal_Rpi5CarGptTestSupport.cpp
        simulation/src/xHal_Rpi5CarGptSimulationArguments.cpp)
    target_include_directories(xWalkSpeechToTextTest PRIVATE test/include simulation/include)
    target_compile_definitions(xWalkSpeechToTextTest PRIVATE ${XWALK_GPT_TEST_DEFINITIONS})
    target_link_libraries(xWalkSpeechToTextTest PRIVATE xWalkGPT xWalkTrace)
    add_dependencies(xWalkSpeechToTextTest xWalkGptTestTraceConfig)
    add_test(NAME xWalkSpeechToTextHostTest COMMAND xWalkSpeechToTextTest)
    set_tests_properties(xWalkSpeechToTextHostTest PROPERTIES LABELS "host")

    add_executable(xWalkTextToSpeechTest
        test/src/xHal_Rpi5CarTextToSpeechTest.cpp
        test/src/xHal_Rpi5CarGptTestSupport.cpp)
    target_include_directories(xWalkTextToSpeechTest PRIVATE test/include)
    target_compile_definitions(xWalkTextToSpeechTest PRIVATE ${XWALK_GPT_TEST_DEFINITIONS})
    target_link_libraries(xWalkTextToSpeechTest PRIVATE xWalkGPT xWalkTrace)
    add_dependencies(xWalkTextToSpeechTest xWalkGptTestTraceConfig)
    add_test(NAME xWalkTextToSpeechHostTest COMMAND xWalkTextToSpeechTest)
    set_tests_properties(xWalkTextToSpeechHostTest PROPERTIES LABELS "host")

    add_executable(xWalkSpeechToTextAlsaTest
        hardware/test/src/xHal_Rpi5CarSpeechToTextAlsaTest.cpp)
    target_include_directories(xWalkSpeechToTextAlsaTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/test/include)
    target_link_libraries(xWalkSpeechToTextAlsaTest PRIVATE xWalkSpeechToTextAlsa)
    add_test(NAME xWalkSpeechToTextAlsaHostTest COMMAND xWalkSpeechToTextAlsaTest)
    set_tests_properties(xWalkSpeechToTextAlsaHostTest PROPERTIES LABELS "host")

    add_executable(xWalkTextToSpeechAlsaTest
        hardware/test/src/xHal_Rpi5CarTextToSpeechAlsaTest.cpp)
    target_include_directories(xWalkTextToSpeechAlsaTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/test/include)
    target_link_libraries(xWalkTextToSpeechAlsaTest PRIVATE xWalkTextToSpeechAlsa)
    add_test(NAME xWalkTextToSpeechAlsaHostTest COMMAND xWalkTextToSpeechAlsaTest)
    set_tests_properties(xWalkTextToSpeechAlsaHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_GPT_BUILD_HARDWARE_TESTS)
    add_executable(xWalkSpeechToTextHardwareTest
        hardware/test/src/xHal_Rpi5CarSpeechToTextAlsaHardwareTest.cpp)
target_link_libraries(xWalkSpeechToTextHardwareTest PRIVATE xWalkTrace)
    target_link_libraries(xWalkSpeechToTextHardwareTest PRIVATE xWalkSpeechToTextAlsa)
    add_test(NAME xWalkSpeechToTextMicrophoneHardwareTest
        COMMAND xWalkSpeechToTextHardwareTest)
    set_tests_properties(xWalkSpeechToTextMicrophoneHardwareTest PROPERTIES LABELS "hardware")

    add_executable(xWalkTextToSpeechHardwareTest
        test/hardware/src/xHal_Rpi5CarTextToSpeechHardwareTest.cpp)
    target_include_directories(xWalkTextToSpeechHardwareTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/hardware/include)
target_link_libraries(xWalkTextToSpeechHardwareTest PRIVATE xWalkTrace)
    target_link_libraries(xWalkTextToSpeechHardwareTest PRIVATE xWalkTextToSpeechAlsa)
    add_test(NAME xWalkTextToSpeechPlaybackHardwareTest COMMAND xWalkTextToSpeechHardwareTest)
    set_tests_properties(xWalkTextToSpeechPlaybackHardwareTest PROPERTIES LABELS "hardware")
endif()
