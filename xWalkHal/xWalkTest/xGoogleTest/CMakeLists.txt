cmake_minimum_required(VERSION 3.16)

project(xGoogleTest LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/XWalkDependencies.cmake)

find_package(GTest CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

set(XWALK_HAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(XWALK_WORKSPACE_SOURCE_DIRECTORY ${XWALK_HAL_SOURCE_DIRECTORY}/..)
include(${XWALK_WORKSPACE_SOURCE_DIRECTORY}/xWalkLibrary/VoskModel.cmake)

set(XWALK_GOOGLE_TEST_HOST_CONFIG_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/config/test_config.xml)
set(XWALK_GOOGLE_TEST_HOST_CONFIG_OUTPUT
    ${CMAKE_BINARY_DIR}/test_config.xml)
set(XWALK_GOOGLE_TEST_HARDWARE_CONFIG_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/config/hardware_test_config.xml)
set(XWALK_GOOGLE_TEST_HARDWARE_CONFIG_OUTPUT
    ${CMAKE_BINARY_DIR}/hardware_test_config.xml)
configure_file(
    ${XWALK_GOOGLE_TEST_HOST_CONFIG_SOURCE}
    ${XWALK_GOOGLE_TEST_HOST_CONFIG_OUTPUT}
    COPYONLY)
configure_file(
    ${XWALK_GOOGLE_TEST_HARDWARE_CONFIG_SOURCE}
    ${XWALK_GOOGLE_TEST_HARDWARE_CONFIG_OUTPUT}
    COPYONLY)
configure_file(config/xHal_Rpi5CarGoogleTestConfig.yml
    ${CMAKE_BINARY_DIR}/xHal_Rpi5CarGoogleTestConfig.yml @ONLY)

add_executable(xGoogleTest
    src/main.cpp
    src/TestConfig.cpp
    src/TestRunner.cpp
)
set_target_properties(xGoogleTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
target_include_directories(xGoogleTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkPwm/test/include
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkServo/test/include
)
target_compile_features(xGoogleTest PRIVATE cxx_std_17)
if(XWALK_ENABLE_THREAD_SANITIZER)
    target_compile_definitions(xGoogleTest PRIVATE
        XWALK_GOOGLE_TEST_THREAD_SANITIZER=1)
endif()

function(xwalk_add_legacy_test_source object_target entry_point source_path)
    add_library(${object_target} OBJECT ${source_path})
    target_compile_features(${object_target} PRIVATE cxx_std_17)
    target_compile_definitions(${object_target} PRIVATE main=${entry_point})
    target_include_directories(${object_target} PRIVATE
        ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkPwm/test/include
        ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkServo/test/include)
    target_link_libraries(${object_target} PRIVATE ${ARGN})
    target_sources(xGoogleTest PRIVATE $<TARGET_OBJECTS:${object_target}>)
endfunction()

xwalk_add_legacy_test_source(xGoogleTestI2c xWalkI2cLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkI2c/test/xHal_Rpi5CarI2cTest.cpp
    xWalkI2c)
xwalk_add_legacy_test_source(xGoogleTestSpi xWalkSpiLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkSpi/test/src/xHal_Rpi5CarSpiTest.cpp
    xWalkSpi)
xwalk_add_legacy_test_source(xGoogleTestGpio xWalkGpioLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkGpio/test/xHal_Rpi5CarGpioTest.cpp
    xWalkGpio)
xwalk_add_legacy_test_source(xGoogleTestAudioAlsa xWalkAudioAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkAudio/hardware/test/src/xHal_Rpi5CarAudioAlsaTest.cpp
    xWalkAudioAlsa)
xwalk_add_legacy_test_source(xGoogleTestConfig xWalkConfigLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkConfig/test/src/xHal_Rpi5CarConfigTest.cpp
    xWalkConfig)
xwalk_add_legacy_test_source(xGoogleTestConfigStore xWalkConfigStoreLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkConfig/test/src/xHal_Rpi5CarConfigStoreTest.cpp
    xWalkConfig)
xwalk_add_legacy_test_source(xGoogleTestTrace xWalkTraceLegacyMain
    ${XWALK_WORKSPACE_SOURCE_DIRECTORY}/xWalkTrace/test/src/xHal_Rpi5CarTraceTest.cpp
    xWalkTrace)
xwalk_add_legacy_test_source(xGoogleTestUtils xWalkUtilsLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkUtils/test/src/xHal_Rpi5CarUtilsTest.cpp
    xWalkUtils)
xwalk_add_legacy_test_source(xGoogleTestUtilsLinux xWalkUtilsLinuxLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkUtils/hardware/test/src/xHal_Rpi5CarUtilsLinuxTest.cpp
    xWalkUtilsLinux)
xwalk_add_legacy_test_source(xGoogleTestLanguageModel xWalkLanguageModelLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkLanguageModel/test/src/xHal_Rpi5CarLanguageModelTest.cpp
    xWalkLanguageModel)
xwalk_add_legacy_test_source(xGoogleTestLanguageModelOllama
    xWalkLanguageModelOllamaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkLanguageModel/hardware/test/src/xHal_Rpi5CarLanguageModelOllamaTest.cpp
    xWalkLanguageModelOllama)
xwalk_add_legacy_test_source(xGoogleTestMusic xWalkMusicLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkMusic/test/src/xHal_Rpi5CarMusicTest.cpp
    xWalkMusic)
xwalk_add_legacy_test_source(xGoogleTestMusicAlsa xWalkMusicAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkMusic/hardware/test/src/xHal_Rpi5CarMusicAlsaTest.cpp
    xWalkMusicAlsa)
xwalk_add_legacy_test_source(xGoogleTestSpeaker xWalkSpeakerLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkSpeaker/test/src/xHal_Rpi5CarSpeakerTest.cpp
    xWalkSpeaker)
xwalk_add_legacy_test_source(xGoogleTestSpeakerAlsa xWalkSpeakerAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkSpeaker/hardware/test/src/xHal_Rpi5CarSpeakerAlsaTest.cpp
    xWalkSpeakerAlsa)
xwalk_add_legacy_test_source(xGoogleTestPwm xWalkPwmLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkPwm/test/src/xHal_Rpi5CarPwmTestMain.cpp
    xWalkPwm)
xwalk_add_legacy_test_source(xGoogleTestAdc xWalkAdcLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkAdc/test/src/xHal_Rpi5CarAdcTest.cpp
    xWalkAdc)
xwalk_add_legacy_test_source(xGoogleTestServo xWalkServoLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkServo/test/src/xHal_Rpi5CarServoTestMain.cpp
    xWalkServo)
xwalk_add_legacy_test_source(xGoogleTestAdxl345 xWalkAdxl345LegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkAdxl345/test/src/xHal_Rpi5CarAdxl345Test.cpp
    xWalkAdxl345)
xwalk_add_legacy_test_source(xGoogleTestLineTracker xWalkLineTrackerLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkLineTracker/test/src/xHal_Rpi5CarLineTrackerTest.cpp
    xWalkLineTracker)
xwalk_add_legacy_test_source(xGoogleTestUltrasonic xWalkUltrasonicLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkUltrasonic/test/src/xHal_Rpi5CarUltrasonicTest.cpp
    xWalkUltrasonic)
xwalk_add_legacy_test_source(xGoogleTestMotor xWalkMotorLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkMotor/test/src/xHal_Rpi5CarMotorTest.cpp
    xWalkMotor)
xwalk_add_legacy_test_source(xGoogleTestLed xWalkLedLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkLed/test/src/xHal_Rpi5CarLedTest.cpp
    xWalkLed)
xwalk_add_legacy_test_source(xGoogleTestRgbLed xWalkRgbLedLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkLed/test/src/xHal_Rpi5CarRgbLedTest.cpp
    xWalkLed)
xwalk_add_legacy_test_source(xGoogleTestBuzzer xWalkBuzzerLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkBuzzer/test/src/xHal_Rpi5CarBuzzerTest.cpp
    xWalkBuzzer)
xwalk_add_legacy_test_source(xGoogleTestCamera xWalkCameraLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkCamera/test/src/xHal_Rpi5CarCameraTest.cpp
    xWalkCamera)
xwalk_add_legacy_test_source(xGoogleTestUserButton xWalkUserButtonLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkUserButton/test/src/xHal_Rpi5CarUserButtonTest.cpp
    xWalkUserButton)
xwalk_add_legacy_test_source(xGoogleTestBoardControl xWalkBoardControlLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkBoardControl/test/src/xHal_Rpi5CarBoardControlTest.cpp
    xWalkBoardControl)
xwalk_add_legacy_test_source(xGoogleTestDevice xWalkDeviceLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkBoardControl/test/src/xHal_Rpi5CarDeviceTest.cpp
    xWalkBoardControl)
xwalk_add_legacy_test_source(xGoogleTestFirmwareInfo xWalkFirmwareInfoLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkBoardControl/test/src/xHal_Rpi5CarFirmwareInfoTest.cpp
    xWalkBoardControl)
xwalk_add_legacy_test_source(xGoogleTestRobot xWalkRobotLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkRobot/test/src/xHal_Rpi5CarRobotTest.cpp
    xWalkRobot)
xwalk_add_legacy_test_source(xGoogleTestSpeechToText xWalkSpeechToTextLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkGPT/test/src/xHal_Rpi5CarSpeechToTextTest.cpp
    xWalkGPT)
xwalk_add_legacy_test_source(xGoogleTestTextToSpeech xWalkTextToSpeechLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkGPT/test/src/xHal_Rpi5CarTextToSpeechTest.cpp
    xWalkGPT)
xwalk_add_legacy_test_source(xGoogleTestSpeechToTextAlsa xWalkSpeechToTextAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkGPT/hardware/test/src/xHal_Rpi5CarSpeechToTextAlsaTest.cpp
    xWalkSpeechToTextAlsa)
xwalk_add_legacy_test_source(xGoogleTestTextToSpeechAlsa xWalkTextToSpeechAlsaLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkGPT/hardware/test/src/xHal_Rpi5CarTextToSpeechAlsaTest.cpp
    xWalkTextToSpeechAlsa)
xwalk_add_legacy_test_source(xGoogleTestVoiceAssistant xWalkVoiceAssistantLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkVoiceAssistant/test/src/xHal_Rpi5CarVoiceAssistantTest.cpp
    xWalkVoiceAssistant)
xwalk_add_legacy_test_source(xGoogleTestVoiceAssistantBackends
    xWalkVoiceAssistantBackendsLegacyMain
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkVoiceAssistant/hardware/test/src/xHal_Rpi5CarVoiceAssistantBackendsTest.cpp
    xWalkVoiceAssistant xWalkSpeechToTextAlsa xWalkTextToSpeechAlsa
    xWalkLanguageModelOllama)
xwalk_add_legacy_test_source(xGoogleTestButtonEventSequence
    xWalkButtonEventSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarButtonEventSequenceTest.cpp
    xWalkButtonEventSequence)
xwalk_add_legacy_test_source(xGoogleTestInitAnglesSequence
    xWalkInitAnglesSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarInitAnglesSequenceTest.cpp
    xWalkInitAnglesSequence)
xwalk_add_legacy_test_source(xGoogleTestRobotHat5MotorSequence
    xWalkRobotHat5MotorSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarRobotHat5MotorSequenceTest.cpp
    xWalkRobotHat5MotorSequence)
xwalk_add_legacy_test_source(xGoogleTestMotorSequence
    xWalkMotorSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarMotorSequenceTest.cpp
    xWalkMotorSequence)
xwalk_add_legacy_test_source(xGoogleTestServoHatSequence
    xWalkServoHatSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarServoHatSequenceTest.cpp
    xWalkServoHatSequence)
xwalk_add_legacy_test_source(xGoogleTestServoSequence
    xWalkServoSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarServoSequenceTest.cpp
    xWalkServoSequence)
xwalk_add_legacy_test_source(xGoogleTestPiperStreamSequence
    xWalkPiperStreamSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarPiperStreamSequenceTest.cpp
    xWalkPiperStreamSequence)
xwalk_add_legacy_test_source(xGoogleTestToneSequence
    xWalkToneSequenceHostTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../xSequenceTest/core/src/xHal_Rpi5CarToneSequenceTest.cpp
    xWalkToneSequence)

target_sources(xGoogleTest PRIVATE
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkPwm/test/src/xHal_Rpi5CarPwmTestAddress.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkPwm/test/src/xHal_Rpi5CarPwmTestI2c.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkPwm/test/src/xHal_Rpi5CarPwmTestI2cLifecycle.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkPwm/test/src/xHal_Rpi5CarPwmTestOutput.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkPwm/test/src/xHal_Rpi5CarPwmTestTimer.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkPwm/test/src/xHal_Rpi5CarPwmTestValidation.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkServo/test/src/xHal_Rpi5CarServoTestAngle.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkServo/test/src/xHal_Rpi5CarServoTestI2c.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkServo/test/src/xHal_Rpi5CarServoTestI2cLifecycle.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkServo/test/src/xHal_Rpi5CarServoTestInitialization.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkServo/test/src/xHal_Rpi5CarServoTestPulse.cpp
    ${XWALK_HAL_SOURCE_DIRECTORY}/xWalkServo/test/src/xHal_Rpi5CarServoTestValidation.cpp
)

target_link_libraries(xGoogleTest PRIVATE
    GTest::gtest
    tinyxml2::tinyxml2
    yaml-cpp::yaml-cpp
    xWalkHal
    xWalkAudioAlsa
    xWalkButtonEventSequence
    xWalkInitAnglesSequence
    xWalkMotorSequence
    xWalkPiperStreamSequence
    xWalkRobotHat5MotorSequence
    xWalkServoHatSequence
    xWalkServoSequence
    xWalkToneSequence
    xWalkLanguageModelOllama
    xWalkMusicAlsa
    xWalkSpeakerAlsa
    xWalkSpeechToTextAlsa
    xWalkTextToSpeechAlsa
    xWalkUtilsLinux
)

if(XWALK_HAL_BUILD_RPI)
    set(XWALK_HARDWARE_TEST_TARGETS
        xWalkAdcHardwareTest
        xWalkAdxl345HardwareTest
        xWalkAudioAlsaHardwareTest
        xWalkBoardControlHardwareTest
        xWalkDeviceHardwareTest
        xWalkFirmwareInfoHardwareTest
        xWalkBuzzerHardwareTest
        xWalkCameraHardwareTest
        xWalkGpioHardwareTest
        xWalkSpeechToTextHardwareTest
        xWalkTextToSpeechHardwareTest
        xWalkI2cLinuxHardwareTest
        xWalkLanguageModelHardwareTest
        xWalkLedHardwareTest
        xWalkRgbLedHardwareTest
        xWalkMotorHardwareTest
        xWalkMusicAlsaHardwareTest
        xWalkPwmHardwareTest
        xWalkServoHardwareTest
        xWalkSpeakerAlsaHardwareTest
        xWalkSpiLinuxHardwareTest
        xWalkTraceHardwareTest
        xWalkUserButtonHardwareTest
        xWalkUtilsLinuxHardwareTest
        xWalkVoiceAssistantHardwareTest
        xSequenceTest
    )
    add_dependencies(xGoogleTest ${XWALK_HARDWARE_TEST_TARGETS})
    target_compile_definitions(xGoogleTest PRIVATE
        XWALK_GOOGLE_TEST_HARDWARE_PROFILE=1
        XWALK_HARDWARE_ADC_TEST="$<TARGET_FILE:xWalkAdcHardwareTest>"
        XWALK_HARDWARE_ADXL345_TEST="$<TARGET_FILE:xWalkAdxl345HardwareTest>"
        XWALK_HARDWARE_AUDIO_TEST="$<TARGET_FILE:xWalkAudioAlsaHardwareTest>"
        XWALK_HARDWARE_BOARD_CONTROL_TEST="$<TARGET_FILE:xWalkBoardControlHardwareTest>"
        XWALK_HARDWARE_DEVICE_TEST="$<TARGET_FILE:xWalkDeviceHardwareTest>"
        XWALK_HARDWARE_FIRMWARE_TEST="$<TARGET_FILE:xWalkFirmwareInfoHardwareTest>"
        XWALK_HARDWARE_BUZZER_TEST="$<TARGET_FILE:xWalkBuzzerHardwareTest>"
        XWALK_HARDWARE_CAMERA_TEST="$<TARGET_FILE:xWalkCameraHardwareTest>"
        XWALK_HARDWARE_GPIO_TEST="$<TARGET_FILE:xWalkGpioHardwareTest>"
        XWALK_HARDWARE_GPT_STT_TEST="$<TARGET_FILE:xWalkSpeechToTextHardwareTest>"
        XWALK_HARDWARE_GPT_TTS_TEST="$<TARGET_FILE:xWalkTextToSpeechHardwareTest>"
        XWALK_HARDWARE_I2C_TEST="$<TARGET_FILE:xWalkI2cLinuxHardwareTest>"
        XWALK_HARDWARE_LANGUAGE_MODEL_TEST="$<TARGET_FILE:xWalkLanguageModelHardwareTest>"
        XWALK_HARDWARE_LED_TEST="$<TARGET_FILE:xWalkLedHardwareTest>"
        XWALK_HARDWARE_RGB_LED_TEST="$<TARGET_FILE:xWalkRgbLedHardwareTest>"
        XWALK_HARDWARE_MOTOR_TEST="$<TARGET_FILE:xWalkMotorHardwareTest>"
        XWALK_HARDWARE_MUSIC_TEST="$<TARGET_FILE:xWalkMusicAlsaHardwareTest>"
        XWALK_HARDWARE_PWM_TEST="$<TARGET_FILE:xWalkPwmHardwareTest>"
        XWALK_HARDWARE_SERVO_TEST="$<TARGET_FILE:xWalkServoHardwareTest>"
        XWALK_HARDWARE_SPEAKER_TEST="$<TARGET_FILE:xWalkSpeakerAlsaHardwareTest>"
        XWALK_HARDWARE_SPI_TEST="$<TARGET_FILE:xWalkSpiLinuxHardwareTest>"
        XWALK_HARDWARE_TRACE_TEST="$<TARGET_FILE:xWalkTraceHardwareTest>"
        XWALK_HARDWARE_USER_BUTTON_TEST="$<TARGET_FILE:xWalkUserButtonHardwareTest>"
        XWALK_HARDWARE_UTILS_TEST="$<TARGET_FILE:xWalkUtilsLinuxHardwareTest>"
        XWALK_HARDWARE_VOICE_ASSISTANT_TEST="$<TARGET_FILE:xWalkVoiceAssistantHardwareTest>"
        XWALK_HARDWARE_SEQUENCE_TEST="$<TARGET_FILE:xSequenceTest>"
    )
endif()

if(XWALK_HAL_BUILD_HOST)
    add_test(NAME xGoogleTest COMMAND $<TARGET_FILE:xGoogleTest>)
    set_tests_properties(xGoogleTest PROPERTIES
        LABELS "host;unit"
        TIMEOUT 120)
endif()
