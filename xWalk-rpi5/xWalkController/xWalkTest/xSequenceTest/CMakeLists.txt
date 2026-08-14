cmake_minimum_required(VERSION 3.16)

project(xCliSequenceTest LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/XWalkDependencies.cmake)
find_package(GTest CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)

if(NOT TARGET xWalkController)
    message(FATAL_ERROR "xCliSequenceTest requires the xWalkController target")
endif()

set(XWALK_CLI_SEQUENCE_TEST_CONFIG_OUTPUT
    ${CMAKE_BINARY_DIR}/xCliSequenceTestConfig.xml)
set(XWALK_CLI_SEQUENCE_HARDWARE_TEST_CONFIG_OUTPUT
    ${CMAKE_BINARY_DIR}/xCliSequenceHardwareTestConfig.xml)
configure_file(config/test_config.xml
    ${XWALK_CLI_SEQUENCE_TEST_CONFIG_OUTPUT} COPYONLY)
configure_file(config/hardware_test_config.xml
    ${XWALK_CLI_SEQUENCE_HARDWARE_TEST_CONFIG_OUTPUT} COPYONLY)

add_library(xWalkCliControllerSequence STATIC
    core/src/xControllerSequence.cpp)
target_link_libraries(xWalkCliControllerSequence PRIVATE xWalkTrace)
target_include_directories(xWalkCliControllerSequence PUBLIC core/include)
target_compile_features(xWalkCliControllerSequence PUBLIC cxx_std_17)
target_link_libraries(xWalkCliControllerSequence PUBLIC xWalkController)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkCliControllerSequence PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_executable(xCliSequenceTest
    main.cpp
    ../xGoogleTest/src/xCliTestConfig.cpp
    core/src/xControllerSequenceTest.cpp
    core/src/xControllerCommandTestSupport.cpp
    core/src/xHelpSequenceTest.cpp
    core/src/xDoctorSequenceTest.cpp
    core/src/xServoZeroingSequenceTest.cpp
    core/src/xMoveSequenceTest.cpp
    core/src/xKeyboardControlSequenceTest.cpp
    core/src/xObstacleAvoidanceSequenceTest.cpp
    core/src/xCliffDetectionSequenceTest.cpp
    core/src/xComputerVisionSequenceTest.cpp
    core/src/xFaceTrackingSequenceTest.cpp
    core/src/xBullFightSequenceTest.cpp
    core/src/xTreasureHuntSequenceTest.cpp
    core/src/xVideoRecordingSequenceTest.cpp
    core/src/xVideoCarSequenceTest.cpp
    core/src/xAppControlSequenceTest.cpp
    core/src/xSoundBackgroundMusicSequenceTest.cpp
    core/src/xTurnSequenceTest.cpp
    core/src/xCameraSequenceTest.cpp
    core/src/xSensorSequenceTest.cpp
    core/src/xSpiSequenceTest.cpp
    core/src/xLineTrackingSequenceTest.cpp
    core/src/xSelfDriveSequenceTest.cpp
    core/src/xSoundSequenceTest.cpp
    core/src/xVoiceChatSequenceTest.cpp
    core/src/xVoiceActiveCarSequenceTest.cpp
    core/src/xVoiceActiveCarGptSequenceTest.cpp
    core/src/xGptCarSequenceTest.cpp
    core/src/xVoiceControlledCarSequenceTest.cpp
    core/src/xVoicePromptCarSequenceTest.cpp
    core/src/xStorytellingRobotSequenceTest.cpp
    core/src/xTextVisionTalkSequenceTest.cpp
    core/src/xOnlineLlmSequenceTest.cpp
    core/src/xCalibrationSequenceTest.cpp)
target_link_libraries(xCliSequenceTest PRIVATE xWalkTrace)
set_target_properties(xCliSequenceTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
target_include_directories(xCliSequenceTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../xGoogleTest/include)
target_compile_features(xCliSequenceTest PRIVATE cxx_std_17)
target_compile_definitions(xCliSequenceTest PRIVATE
    XWALK_TEST_SOUND_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkAudioResources/sounds"
    XWALK_TEST_MUSIC_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkAudioResources/music"
    XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY="${XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY}")
target_link_libraries(xCliSequenceTest PRIVATE
    GTest::gtest
    tinyxml2::tinyxml2
    xWalkCliControllerSequence
    xWalkVoiceActiveCarGpt
    xWalkGptCar)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xCliSequenceTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_test(NAME xCliSequenceTestHostTest COMMAND xCliSequenceTest)
set_tests_properties(xCliSequenceTestHostTest PROPERTIES
    LABELS "host;sequence;cli"
    RUN_SERIAL TRUE)
