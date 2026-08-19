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
    set(XWALK_GPT_BUILD_PIPER_PROVIDER ON CACHE BOOL
        "Boot requires the Piper provider" FORCE)
    set(XWALK_GPT_BUILD_PICO2WAVE_PROVIDER ON CACHE BOOL
        "Boot requires the Pico2Wave provider" FORCE)
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
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkSelfDrive
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSelfDrive)
endif()

if(NOT TARGET xWalkServoZeroing)
    set(XWALK_SERVO_ZEROING_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCalibration/xWalkServoZeroing
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkServoZeroing)
endif()

if(NOT TARGET xWalkLineTracking)
    set(XWALK_LINE_TRACKING_BUILD_HOST_TESTS OFF CACHE BOOL "Boot dependency setting" FORCE)
    set(XWALK_LINE_TRACKING_BUILD_HARDWARE_TESTS ${XWALK_BOOT_BUILD_RPI_BACKEND}
        CACHE BOOL "Boot RPi dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkLineTracking
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLineTracking)
endif()

if(NOT TARGET xWalkComputerVision)
    set(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND ${XWALK_BOOT_BUILD_RPI_BACKEND}
        CACHE BOOL "Boot OpenCV dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVision/xWalkComputerVision
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkComputerVision)
endif()

if(NOT TARGET xWalkFaceTracking)
    set(XWALK_FACE_TRACKING_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVision/xWalkFaceTracking
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkFaceTracking)
endif()

if(NOT TARGET xWalkBullFight)
    set(XWALK_BULL_FIGHT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVision/xWalkBullFight
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkBullFight)
endif()

if(NOT TARGET xWalkVideoRecording)
    set(XWALK_VIDEO_RECORDING_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    set(XWALK_VIDEO_RECORDING_BUILD_OPENCV_BACKEND
        ${XWALK_BOOT_BUILD_RPI_BACKEND} CACHE BOOL
        "Boot OpenCV video dependency setting" FORCE)
    set(XWALK_VIDEO_RECORDING_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Boot video hardware-test dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVision/xWalkVideoRecording
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVideoRecording)
endif()

if(NOT TARGET xWalkVideoCar)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVision/xWalkVideoCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVideoCar)
endif()

if(NOT TARGET xWalkAppControl)
    set(XWALK_APP_CONTROL_BUILD_WEBSOCKET_BACKEND
        ${XWALK_BOOT_BUILD_RPI_BACKEND} CACHE BOOL
        "Boot app-control transport dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkConnectivity/xWalkAppControl
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkAppControl)
endif()

if(NOT TARGET xWalkSoundBackgroundMusic)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkMedia/xWalkSoundBackgroundMusic
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSoundBackgroundMusic)
endif()

if(NOT TARGET xWalkLocalVoiceChatbot)
    set(XWALK_LOCAL_VOICE_CHATBOT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVoice/xWalkLocalVoiceChatbot
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLocalVoiceChatbot)
endif()

if(NOT TARGET xWalkVoiceActiveCar)
    set(XWALK_VOICE_ACTIVE_CAR_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVoice/xWalkVoiceActiveCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceActiveCar)
endif()

if(NOT TARGET xWalkVoicePromptCar)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVoice/xWalkVoicePromptCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoicePromptCar)
endif()

if(NOT TARGET xWalkStorytellingRobot)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVoice/xWalkStorytellingRobot
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkStorytellingRobot)
endif()

if(NOT TARGET xWalkVoiceControlledCar)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVoice/xWalkVoiceControlledCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceControlledCar)
endif()

if(NOT TARGET xWalkTextVisionTalk)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVoice/xWalkTextVisionTalk
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTextVisionTalk)
endif()

if(NOT TARGET xWalkOnlineLlmTest)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVoice/xWalkOnlineLlmTest
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkOnlineLlmTest)
endif()

if(NOT TARGET xWalkVoiceActiveCarGpt)
    set(XWALK_VOICE_ACTIVE_CAR_GPT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVoice/xWalkVoiceActiveCarGpt
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceActiveCarGpt)
endif()

if(NOT TARGET xWalkGptCar)
    set(XWALK_GPT_CAR_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVoice/xWalkGptCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkGptCar)
endif()

if(NOT TARGET xWalkCameraCapture)
    set(XWALK_CAMERA_CAPTURE_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVision/xWalkCameraCapture
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCameraCapture)
endif()

if(NOT TARGET xWalkTreasureHunt)
    set(XWALK_TREASURE_HUNT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVision/xWalkTreasureHunt
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTreasureHunt)
endif()

if(NOT TARGET xWalkSpiTransfer)
    set(XWALK_SPI_TRANSFER_BUILD_HOST_TESTS OFF CACHE BOOL
        "Boot dependency setting" FORCE)
    set(XWALK_SPI_BUILD_LINUX_BACKEND ${XWALK_BOOT_BUILD_RPI_BACKEND}
        CACHE BOOL "Boot Linux SPI dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkConnectivity/xWalkSpiTransfer
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSpiTransfer)
endif()

add_library(xWalkBoot STATIC
    core/src/xAgent_Rpi5CarBoot.cpp
    stub/src/xAgent_Rpi5CarBootHostStub.cpp)
target_link_libraries(xWalkBoot PRIVATE xWalkTrace)
add_library(xWalk::Boot ALIAS xWalkBoot)
target_include_directories(xWalkBoot PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/stub/include)
target_compile_features(xWalkBoot PUBLIC cxx_std_17)
target_link_libraries(xWalkBoot PUBLIC
    xWalkLineTracking xWalkSelfDrive xWalkLocalVoiceChatbot xWalkVoiceActiveCar
    xWalkVoicePromptCar
    xWalkStorytellingRobot
    xWalkVoiceControlledCar
    xWalkTextVisionTalk
    xWalkOnlineLlmTest
    xWalkVoiceActiveCarGpt xWalkGptCar xWalkCameraCapture)
target_link_libraries(xWalkBoot PUBLIC xWalkServoZeroing)
target_link_libraries(xWalkBoot PUBLIC
    xWalkComputerVision xWalkFaceTracking xWalkBullFight xWalkTreasureHunt
    xWalkVideoRecording xWalkVideoCar xWalkAppControl
    xWalkSoundBackgroundMusic xWalkSpiTransfer)

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
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_executable(xWalkDoctorLinuxHostTest
            hardware/src/xAgent_Rpi5CarDoctorAssessment.cpp
            hardware/src/xAgent_Rpi5CarDoctorLinuxExecutable.cpp
            hardware/test/src/xAgent_Rpi5CarDoctorLinuxTest.cpp
            hardware/test/src/xAgent_Rpi5CarDoctorLinuxTestSupport.cpp)
        target_include_directories(xWalkDoctorLinuxHostTest PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include
            ${CMAKE_CURRENT_SOURCE_DIR}/hardware/test/include)
        target_link_libraries(xWalkDoctorLinuxHostTest PRIVATE xWalkLibraryCommon)
        target_compile_features(xWalkDoctorLinuxHostTest PRIVATE cxx_std_17)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(xWalkDoctorLinuxHostTest PRIVATE
                -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
        endif()
        add_test(NAME xWalkDoctorLinuxExecutableDiscoveryTest COMMAND xWalkDoctorLinuxHostTest)
        set_tests_properties(xWalkDoctorLinuxExecutableDiscoveryTest PROPERTIES LABELS "host")
    endif()
endif()

if(XWALK_BOOT_BUILD_RPI_BACKEND)
    if(NOT TARGET xWalkBoardControl OR NOT TARGET xWalkI2cLinux OR
        NOT TARGET xWalkGpioLinux OR NOT TARGET xWalkMusicAlsa OR
        NOT TARGET xWalkMusicSndFileDecoder OR
        NOT TARGET xWalkSpeechToTextAlsa OR
        NOT TARGET xWalkTextToSpeechAlsa OR
        NOT TARGET xWalkSpeechRecognizerVosk OR
        NOT TARGET xWalkTextToSpeechEspeak OR
        NOT TARGET xWalkTextToSpeechPiper OR
        NOT TARGET xWalkTextToSpeechPico2Wave OR
        NOT TARGET xWalkLanguageModelOllama OR
        NOT TARGET xWalkCameraLinux OR NOT TARGET xWalkSpiLinux OR
        NOT TARGET xWalkComputerVisionOpenCv OR
        NOT TARGET xWalkVideoRecordingOpenCv OR
        NOT TARGET xWalkAppControlWebSocket)
        message(FATAL_ERROR "xWalkBoot Raspberry Pi dependencies are incomplete")
    endif()
    add_library(xWalkBootRpi STATIC
        hardware/src/xAgent_Rpi5CarDoctorAssessment.cpp
        hardware/src/xAgent_Rpi5CarDoctorLinux.cpp
        hardware/src/xAgent_Rpi5CarDoctorLinuxExecutable.cpp
        hardware/src/xAgent_Rpi5CarBootRpi.cpp
        hardware/src/xAgent_Rpi5CarBootRpiAppControl.cpp
        hardware/src/xAgent_Rpi5CarBootRpiBase.cpp
        hardware/src/xAgent_Rpi5CarBootRpiBoard.cpp
        hardware/src/xAgent_Rpi5CarBootRpiBullFight.cpp
        hardware/src/xAgent_Rpi5CarBootRpiComputerVision.cpp
        hardware/src/xAgent_Rpi5CarBootRpiDoctor.cpp
        hardware/src/xAgent_Rpi5CarBootRpiFaceTracking.cpp
        hardware/src/xAgent_Rpi5CarBootRpiGptCar.cpp
        hardware/src/xAgent_Rpi5CarBootRpiLineTracking.cpp
        hardware/src/xAgent_Rpi5CarBootRpiOnlineLlmTest.cpp
        hardware/src/xAgent_Rpi5CarBootRpiSelfDrive.cpp
        hardware/src/xAgent_Rpi5CarBootRpiServoZeroing.cpp
        hardware/src/xAgent_Rpi5CarBootRpiSound.cpp
        hardware/src/xAgent_Rpi5CarBootRpiSoundBackgroundMusic.cpp
        hardware/src/xAgent_Rpi5CarBootRpiSpiTransfer.cpp
        hardware/src/xAgent_Rpi5CarBootRpiStorytellingRobot.cpp
        hardware/src/xAgent_Rpi5CarBootRpiTextVisionTalk.cpp
        hardware/src/xAgent_Rpi5CarBootRpiTreasureHunt.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVehicle.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVehicleMode.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVideoCar.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVideoRecording.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVoiceActiveCar.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVoiceActiveCarGpt.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVoiceActiveMode.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVoiceChat.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVoiceControlledCar.cpp
        hardware/src/xAgent_Rpi5CarBootRpiVoicePromptCar.cpp)
target_link_libraries(xWalkBootRpi PRIVATE xWalkTrace)
    add_library(xWalk::BootRpi ALIAS xWalkBootRpi)
    target_include_directories(xWalkBootRpi PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkBootRpi PUBLIC cxx_std_17)
    target_link_libraries(xWalkBootRpi
        PUBLIC xWalkBoot
        PRIVATE
            xWalkBoardControl
            xWalkConfig
            xWalkI2cLinux
            xWalkGpioLinux
            xWalkMusicAlsa
            xWalkMusicSndFileDecoder
            xWalkSpeechRecognizerVosk
            xWalkTextToSpeechEspeak
            xWalkTextToSpeechPiper
            xWalkTextToSpeechPico2Wave
            xWalkLanguageModelOllama
            xWalkCameraLinux
            xWalkComputerVisionOpenCv
            xWalkVideoRecordingOpenCv)
    target_link_libraries(xWalkBootRpi PRIVATE xWalkAppControlWebSocket)
    target_link_libraries(xWalkBootRpi PRIVATE xWalkSpiLinux)
    target_link_libraries(xWalkBootRpi PRIVATE ${CMAKE_DL_LIBS})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkBootRpi PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()
