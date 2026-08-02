cmake_minimum_required(VERSION 3.16)

project(xWalkBoot LANGUAGES CXX)

option(XWALK_BOOT_BUILD_HOST_TESTS "Build the xWalkBoot device-free host test" OFF)
option(XWALK_BOOT_BUILD_RPI_BACKEND "Build the xWalkBoot Raspberry Pi composition" OFF)

if(XWALK_BOOT_BUILD_RPI_BACKEND)
    set(XWALK_GPT_BUILD_STT_ALSA_BACKEND ON CACHE BOOL
        "Boot requires ALSA speech capture" FORCE)
    set(XWALK_GPT_BUILD_TTS_ALSA_BACKEND ON CACHE BOOL
        "Boot requires ALSA speech playback" FORCE)
    set(XWALK_GPT_BUILD_VOSK_PROVIDER ON CACHE BOOL
        "Boot requires the Vosk provider" FORCE)
    set(XWALK_GPT_BUILD_ESPEAK_PROVIDER ON CACHE BOOL
        "Boot requires the Espeak provider" FORCE)
    set(XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER ON CACHE BOOL
        "Boot requires the Ollama provider" FORCE)
    set(XWALK_CAMERA_BUILD_LINUX_BACKEND ON CACHE BOOL
        "Boot requires Linux camera capture" FORCE)
endif()

if(XWALK_BOOT_BUILD_RPI_BACKEND AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkBoot Raspberry Pi composition requires Linux")
endif()

if(NOT TARGET xWalkSelfDrive)
    set(XWALK_SELF_DRIVE_BUILD_HOST_TESTS OFF CACHE BOOL "Boot dependency setting" FORCE)
    set(XWALK_SELF_DRIVE_BUILD_HARDWARE_TESTS ${XWALK_BOOT_BUILD_RPI_BACKEND}
        CACHE BOOL "Boot RPi dependency setting" FORCE)
    set(XWALK_MUSIC_BUILD_ALSA_BACKEND ${XWALK_BOOT_BUILD_RPI_BACKEND}
        CACHE BOOL "Boot audio dependency setting" FORCE)
    set(XWALK_MUSIC_BUILD_SNDFILE_DECODER ${XWALK_BOOT_BUILD_RPI_BACKEND}
        CACHE BOOL "Boot decoder dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkSelfDrive
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSelfDrive)
endif()

if(NOT TARGET xWalkLineTracking)
    set(XWALK_LINE_TRACKING_BUILD_HOST_TESTS OFF CACHE BOOL "Boot dependency setting" FORCE)
    set(XWALK_LINE_TRACKING_BUILD_HARDWARE_TESTS ${XWALK_BOOT_BUILD_RPI_BACKEND}
        CACHE BOOL "Boot RPi dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLineTracking
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLineTracking)
endif()

if(NOT TARGET xWalkLocalVoiceChatbot)
    set(XWALK_LOCAL_VOICE_CHATBOT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLocalVoiceChatbot
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLocalVoiceChatbot)
endif()

if(NOT TARGET xWalkVoiceActiveCar)
    set(XWALK_VOICE_ACTIVE_CAR_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkVoiceActiveCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceActiveCar)
endif()

if(NOT TARGET xWalkVoiceActiveCarGpt)
    set(XWALK_VOICE_ACTIVE_CAR_GPT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkVoiceActiveCarGpt
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceActiveCarGpt)
endif()

if(NOT TARGET xWalkCameraCapture)
    set(XWALK_CAMERA_CAPTURE_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkCameraCapture
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCameraCapture)
endif()

if(NOT TARGET xWalkSpiTransfer)
    set(XWALK_SPI_TRANSFER_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    set(XWALK_SPI_BUILD_LINUX_BACKEND ${XWALK_BOOT_BUILD_RPI_BACKEND}
        CACHE BOOL "Boot Linux SPI dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkSpiTransfer
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSpiTransfer)
endif()

add_library(xWalkBoot STATIC
    core/src/xAgent_Rpi5CarBoot.cpp
    stub/src/xAgent_Rpi5CarBootHostStub.cpp)
add_library(xWalk::Boot ALIAS xWalkBoot)
target_include_directories(xWalkBoot PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/stub/include)
target_compile_features(xWalkBoot PUBLIC cxx_std_17)
target_link_libraries(xWalkBoot PUBLIC
    xWalkLineTracking xWalkSelfDrive xWalkLocalVoiceChatbot xWalkVoiceActiveCar
    xWalkVoiceActiveCarGpt xWalkCameraCapture)
target_link_libraries(xWalkBoot PUBLIC xWalkSpiTransfer)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkBoot PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_BOOT_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkBootTest stub/test/src/xAgent_Rpi5CarBootTest.cpp)
    target_include_directories(xWalkBootTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/stub/test/include)
    target_link_libraries(xWalkBootTest PRIVATE xWalkBoot)
    add_test(NAME xWalkBootHostTest COMMAND xWalkBootTest)
    set_tests_properties(xWalkBootHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_BOOT_BUILD_RPI_BACKEND)
    if(NOT TARGET xWalkBoardControl OR NOT TARGET xWalkI2cLinux OR
        NOT TARGET xWalkGpioLinux OR NOT TARGET xWalkMusicAlsa OR
        NOT TARGET xWalkMusicSndFileDecoder OR
        NOT TARGET xWalkSpeechToTextAlsa OR
        NOT TARGET xWalkTextToSpeechAlsa OR
        NOT TARGET xWalkSpeechRecognizerVosk OR
        NOT TARGET xWalkTextToSpeechEspeak OR
        NOT TARGET xWalkLanguageModelOllama OR
        NOT TARGET xWalkCameraLinux OR NOT TARGET xWalkSpiLinux)
        message(FATAL_ERROR "xWalkBoot Raspberry Pi dependencies are incomplete")
    endif()
    add_library(xWalkBootRpi STATIC
        hardware/src/xAgent_Rpi5CarDoctorLinux.cpp
        hardware/src/xAgent_Rpi5CarBootRpi.cpp)
    add_library(xWalk::BootRpi ALIAS xWalkBootRpi)
    target_include_directories(xWalkBootRpi PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkBootRpi PUBLIC cxx_std_17)
    target_link_libraries(xWalkBootRpi
        PUBLIC xWalkBoot
        PRIVATE
            xWalkBoardControl
            xWalkI2cLinux
            xWalkGpioLinux
            xWalkMusicAlsa
            xWalkMusicSndFileDecoder
            xWalkSpeechRecognizerVosk
            xWalkTextToSpeechEspeak
            xWalkLanguageModelOllama
            xWalkCameraLinux)
    target_link_libraries(xWalkBootRpi PRIVATE xWalkSpiLinux)
    target_link_libraries(xWalkBootRpi PRIVATE ${CMAKE_DL_LIBS})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkBootRpi PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()
