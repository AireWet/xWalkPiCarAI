cmake_minimum_required(VERSION 3.16)

project(xWalkController LANGUAGES CXX)

option(XWALK_CONTROLLER_BUILD_HOST_TESTS "Build PiCar-X CLI host tests" OFF)
option(XWALK_CONTROLLER_BUILD_RPI_TESTS "Build PiCar-X CLI RPi application and test" OFF)

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

set(XWALK_CONTROLLER_HELP_JSON
    "${CMAKE_CURRENT_SOURCE_DIR}/activate/resources/help.json")
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
    "${XWALK_CONTROLLER_GENERATED_INCLUDE_DIRECTORY}/xControllerHelpGenerated.h")
file(MAKE_DIRECTORY "${XWALK_CONTROLLER_GENERATED_INCLUDE_DIRECTORY}")
file(WRITE "${XWALK_CONTROLLER_GENERATED_HELP_HEADER}"
    "#ifndef XCONTROLLER_HELP_GENERATED_H\n"
    "#define XCONTROLLER_HELP_GENERATED_H\n\n"
    "namespace xwalk::ctrl\n{\n\n"
    "inline constexpr ::ctrl::cstring XCONTROLLER_HELP =\n")
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
    "\n} /* namespace xwalk::ctrl */\n\n"
    "#endif /* XCONTROLLER_HELP_GENERATED_H */\n")

if(XWALK_CONTROLLER_BUILD_RPI_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkController hardware mode requires Linux I2C and GPIO support")
endif()

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL "CLI dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS ${XWALK_CONTROLLER_BUILD_RPI_TESTS}
        CACHE BOOL "CLI hardware dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkLineTracking)
    set(XWALK_LINE_TRACKING_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    set(XWALK_LINE_TRACKING_BUILD_HARDWARE_TESTS ${XWALK_CONTROLLER_BUILD_RPI_TESTS}
        CACHE BOOL "CLI hardware dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVehicle/xWalkLineTracking
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLineTracking)
endif()

if(NOT TARGET xWalkGrayscaleCalibration)
    set(XWALK_GRAYSCALE_CALIBRATION_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkCalibration/xWalkGrayscaleCalibration
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkGrayscaleCalibration)
endif()

if(NOT TARGET xWalkServoMotorCalibration)
    set(XWALK_SERVO_MOTOR_CALIBRATION_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkCalibration/xWalkServoMotorCalibration
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkServoMotorCalibration)
endif()

if(NOT TARGET xWalkServoZeroing)
    set(XWALK_SERVO_ZEROING_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkCalibration/xWalkServoZeroing
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkServoZeroing)
endif()

if(NOT TARGET xWalkMoveExample)
    set(XWALK_MOVE_EXAMPLE_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVehicle/xWalkMoveExample
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkMoveExample)
endif()

if(NOT TARGET xWalkKeyboardControl)
    set(XWALK_KEYBOARD_CONTROL_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVehicle/xWalkKeyboardControl
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkKeyboardControl)
endif()

if(NOT TARGET xWalkObstacleAvoidance)
    set(XWALK_OBSTACLE_AVOIDANCE_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVehicle/xWalkObstacleAvoidance
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkObstacleAvoidance)
endif()

if(NOT TARGET xWalkCliffDetection)
    set(XWALK_CLIFF_DETECTION_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVehicle/xWalkCliffDetection
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCliffDetection)
endif()

if(NOT TARGET xWalkComputerVision)
    set(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND ${XWALK_CONTROLLER_BUILD_RPI_TESTS}
        CACHE BOOL "CLI OpenCV dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVision/xWalkComputerVision
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkComputerVision)
endif()

if(NOT TARGET xWalkFaceTracking)
    set(XWALK_FACE_TRACKING_BUILD_HOST_TESTS OFF CACHE BOOL
        "Controller dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVision/xWalkFaceTracking
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkFaceTracking)
endif()

if(NOT TARGET xWalkBullFight)
    set(XWALK_BULL_FIGHT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Controller dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVision/xWalkBullFight
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkBullFight)
endif()

if(NOT TARGET xWalkTreasureHunt)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVision/xWalkTreasureHunt
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTreasureHunt)
endif()

if(NOT TARGET xWalkVideoRecording)
    set(XWALK_VIDEO_RECORDING_BUILD_HOST_TESTS OFF CACHE BOOL
        "Controller dependency setting" FORCE)
    set(XWALK_VIDEO_RECORDING_BUILD_OPENCV_BACKEND
        ${XWALK_CONTROLLER_BUILD_RPI_TESTS} CACHE BOOL
        "Controller OpenCV video dependency setting" FORCE)
    set(XWALK_VIDEO_RECORDING_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Controller video hardware-test dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVision/xWalkVideoRecording
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVideoRecording)
endif()

if(NOT TARGET xWalkVideoCar)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVision/xWalkVideoCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVideoCar)
endif()

if(NOT TARGET xWalkAppControl)
    set(XWALK_APP_CONTROL_BUILD_WEBSOCKET_BACKEND OFF CACHE BOOL
        "Controller app transport dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkConnectivity/xWalkAppControl
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkAppControl)
endif()

if(NOT TARGET xWalkSoundBackgroundMusic)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkMedia/xWalkSoundBackgroundMusic
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSoundBackgroundMusic)
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
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVehicle/xWalkSelfDrive
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSelfDrive)
endif()

if(NOT TARGET xWalkVoicePromptCar)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVoice/xWalkVoicePromptCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoicePromptCar)
endif()

if(NOT TARGET xWalkStorytellingRobot)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVoice/xWalkStorytellingRobot
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkStorytellingRobot)
endif()

if(NOT TARGET xWalkVoiceControlledCar)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVoice/xWalkVoiceControlledCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceControlledCar)
endif()

if(NOT TARGET xWalkTextVisionTalk)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVoice/xWalkTextVisionTalk
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTextVisionTalk)
endif()

if(NOT TARGET xWalkOnlineLlmTest)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVoice/xWalkOnlineLlmTest
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkOnlineLlmTest)
endif()

if(NOT TARGET xWalkGptCar)
    set(XWALK_GPT_CAR_BUILD_HOST_TESTS OFF CACHE BOOL
        "Controller dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkVoice/xWalkGptCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkGptCar)
endif()

if(NOT TARGET xWalkBoot)
    set(XWALK_BOOT_BUILD_HOST_TESTS OFF CACHE BOOL
        "CLI boot dependency setting" FORCE)
    set(XWALK_BOOT_BUILD_RPI_BACKEND ${XWALK_CONTROLLER_BUILD_RPI_TESTS} CACHE BOOL
        "CLI RPi boot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/xWalkPlatform/xWalkBoot
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkBoot)
endif()

add_library(xWalkController STATIC
    activate/src/xControllerCommandRunner.cpp
    activate/src/xControllerPicarxCommands.cpp
    activate/src/xControllerUsage.cpp
    parse/src/xControllerApplicationArguments.cpp
    parse/src/xControllerCommandParser.cpp
    parse/src/xControllerHelpRequest.cpp
    parse/src/xControllerParsing.cpp
    boot/src/xControllerDeploymentConfig.cpp
    ../xWalkHandler/src/xControllerLifecycle.cpp
    ../xWalkHandler/src/calibration/xControllerCalibrationHandler.cpp
    ../xWalkHandler/src/calibration/xControllerCalibrationSupport.cpp
    ../xWalkHandler/src/calibration/xControllerFirstRunVerificationHandler.cpp
    ../xWalkHandler/src/calibration/xControllerServoZeroingHandler.cpp
    ../xWalkHandler/src/common/xControllerCommandSafety.cpp
    ../xWalkHandler/src/connectivity/xControllerAppControlHandler.cpp
    ../xWalkHandler/src/connectivity/xControllerSpiHandler.cpp
    ../xWalkHandler/src/media/xControllerSoundBackgroundMusicHandler.cpp
    ../xWalkHandler/src/media/xControllerSoundHandler.cpp
    ../xWalkHandler/src/platform/xControllerDoctorHandler.cpp
    ../xWalkHandler/src/vehicle/xControllerCliffDetectionHandler.cpp
    ../xWalkHandler/src/vehicle/xControllerKeyboardControlHandler.cpp
    ../xWalkHandler/src/vehicle/xControllerLineTrackingHandler.cpp
    ../xWalkHandler/src/vehicle/xControllerMoveExampleHandler.cpp
    ../xWalkHandler/src/vehicle/xControllerMoveHandler.cpp
    ../xWalkHandler/src/vehicle/xControllerObstacleAvoidanceHandler.cpp
    ../xWalkHandler/src/vehicle/xControllerSelfDriveHandler.cpp
    ../xWalkHandler/src/vehicle/xControllerSensorHandler.cpp
    ../xWalkHandler/src/vehicle/xControllerTurnHandler.cpp
    ../xWalkHandler/src/vision/xControllerBullFightHandler.cpp
    ../xWalkHandler/src/vision/xControllerCameraHandler.cpp
    ../xWalkHandler/src/vision/xControllerComputerVisionHandler.cpp
    ../xWalkHandler/src/vision/xControllerFaceTrackingHandler.cpp
    ../xWalkHandler/src/vision/xControllerTreasureHuntHandler.cpp
    ../xWalkHandler/src/vision/xControllerVideoCarHandler.cpp
    ../xWalkHandler/src/vision/xControllerVideoRecordingHandler.cpp
    ../xWalkHandler/src/voice/xControllerGptCarHandler.cpp
    ../xWalkHandler/src/voice/xControllerOnlineLlmTestHandler.cpp
    ../xWalkHandler/src/voice/xControllerStorytellingRobotHandler.cpp
    ../xWalkHandler/src/voice/xControllerTextVisionTalkHandler.cpp
    ../xWalkHandler/src/voice/xControllerVoiceActiveCarHandler.cpp
    ../xWalkHandler/src/voice/xControllerVoiceChatHandler.cpp
    ../xWalkHandler/src/voice/xControllerVoiceControlledCarHandler.cpp
    ../xWalkHandler/src/voice/xControllerVoicePromptCarHandler.cpp
)
target_link_libraries(xWalkController PRIVATE xWalkTrace)
add_library(xWalk::Controller ALIAS xWalkController)

target_include_directories(xWalkController
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/activate/include
        ${CMAKE_CURRENT_SOURCE_DIR}/boot/include
        ${CMAKE_CURRENT_SOURCE_DIR}/parse/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkHandler/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/common/include
    PRIVATE ${XWALK_CONTROLLER_GENERATED_INCLUDE_DIRECTORY})
target_compile_definitions(xWalkController PRIVATE
    XWALK_CONTROLLER_USE_GENERATED_HELP=1)
target_compile_features(xWalkController PUBLIC cxx_std_17)
target_link_libraries(xWalkController PUBLIC
    xWalkPicarx
    xWalkGrayscaleCalibration
    xWalkServoMotorCalibration
    xWalkServoZeroing
    xWalkMoveExample
    xWalkKeyboardControl
    xWalkObstacleAvoidance
    xWalkCliffDetection
    xWalkComputerVision
    xWalkFaceTracking
    xWalkBullFight
    xWalkTreasureHunt
    xWalkVideoRecording
    xWalkVideoCar
    xWalkAppControl
    xWalkSoundBackgroundMusic
    xWalkLineTracking
    xWalkLocalVoiceChatbot
    xWalkVoicePromptCar
    xWalkStorytellingRobot
    xWalkVoiceControlledCar
    xWalkTextVisionTalk
    xWalkOnlineLlmTest
    xWalkGptCar
    xWalkVoiceActiveCar
    xWalkSelfDrive)
target_link_libraries(xWalkController PUBLIC xWalkSpiTransfer)
target_link_libraries(xWalkController PUBLIC xWalkConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkController PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_CONTROLLER_BUILD_HOST_TESTS OR XWALK_CONTROLLER_BUILD_RPI_TESTS)
    include(CTest)
endif()

if(XWALK_CONTROLLER_BUILD_HOST_TESTS)
    find_package(GTest CONFIG REQUIRED)
    add_executable(xwalk-picarx-control
        boot/src/xControllerApplicationSupport.cpp
        boot/src/xControllerBootMode.cpp
        boot/src/xControllerRunner.cpp
        cli/host/src/xControllerHostApplication.cpp
        cli/host/src/xControllerHostMain.cpp)
    target_compile_definitions(xwalk-picarx-control PRIVATE
        XWALK_RUNTIME_DATA_DIRECTORY="${XWALK_RUNTIME_DATA_DIRECTORY}")
    target_link_libraries(xwalk-picarx-control PRIVATE
        xWalkController
        xWalkBoot
        xWalkTrace)
    add_executable(xWalkControllerTest
        ../xWalkHandler/test/src/xControllerTest.cpp)
    target_include_directories(xWalkControllerTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkHandler/test/include)
    target_compile_definitions(xWalkControllerTest PRIVATE
        XWALK_TEST_SOUND_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAudioResources/sounds"
        XWALK_TEST_MUSIC_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAudioResources/music")
    target_link_libraries(xWalkControllerTest PRIVATE xWalkController)
    add_test(NAME xWalkControllerHostTest COMMAND xWalkControllerTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/cli-test.conf)
    set_tests_properties(xWalkControllerHostTest PROPERTIES LABELS "host")
    add_executable(xWalkAppTest
        boot/src/xControllerApplicationSupport.cpp
        boot/src/xControllerBootMode.cpp
        boot/src/xControllerRunner.cpp
        test/src/xControllerParsingTest.cpp
        test/src/xControllerAppTest.cpp)
    target_include_directories(xWalkAppTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_compile_features(xWalkAppTest PRIVATE cxx_std_17)
    target_compile_definitions(xWalkAppTest PRIVATE
        XWALK_CONTROLLER_TRACE_EXAMPLE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../xWalkConfig")
    target_link_libraries(xWalkAppTest PRIVATE
        GTest::gtest_main
        xWalkController
        xWalkBoot
        xWalkTrace
        xWalkVoiceActiveCarGpt)
    add_dependencies(xWalkAppTest xwalk-picarx-control)
    add_test(NAME xWalkAppHostTest COMMAND xWalkAppTest)
    set_tests_properties(xWalkAppHostTest PROPERTIES
        LABELS "host;deployment;app")
endif()

if(XWALK_CONTROLLER_BUILD_RPI_TESTS)
    if(NOT TARGET xWalkBootRpi)
        message(FATAL_ERROR "xWalkController RPi application requires xWalkBootRpi")
    endif()
    add_executable(xwalk-picarx-control
        boot/src/xControllerApplicationSupport.cpp
        boot/src/xControllerBootMode.cpp
        boot/src/xControllerRunner.cpp
        cli/hardware/src/xControllerMain.cpp)
    target_compile_definitions(xwalk-picarx-control PRIVATE
        XWALK_PICARX_CONFIG_FILE="${XWALK_PICARX_CONFIG_FILE}"
        XWALK_RUNTIME_DATA_DIRECTORY="${XWALK_RUNTIME_DATA_DIRECTORY}")
    target_link_libraries(xwalk-picarx-control PRIVATE
        xWalkController
        xWalkBootRpi
        xWalkTrace
    )
    add_test(NAME xWalkControllerHardwareHelpTest COMMAND xwalk-picarx-control --help)
    set_tests_properties(xWalkControllerHardwareHelpTest PROPERTIES LABELS "hardware")
endif()
