cmake_minimum_required(VERSION 3.16)

project(xSequenceTest LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/XWalkDependencies.cmake)

set(XWALK_SEQUENCE_TEST_CONFIG_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/config)
set(XWALK_SEQUENCE_TEST_YAML_OUTPUT
    ${XWALK_SEQUENCE_TEST_CONFIG_OUTPUT_DIRECTORY}/xHal_Rpi5CarSequenceTestConfig.yml)
file(MAKE_DIRECTORY ${XWALK_SEQUENCE_TEST_CONFIG_OUTPUT_DIRECTORY})
configure_file(config/test_config.xml
    ${XWALK_SEQUENCE_TEST_CONFIG_OUTPUT_DIRECTORY}/test_config.xml COPYONLY)
configure_file(config/hardware_test_config.xml
    ${XWALK_SEQUENCE_TEST_CONFIG_OUTPUT_DIRECTORY}/hardware_test_config.xml COPYONLY)
configure_file(config/xHal_Rpi5CarSequenceTestConfig.yml
    ${XWALK_SEQUENCE_TEST_YAML_OUTPUT} COPYONLY)

add_library(xWalkButtonEventSequence
    core/src/xHal_Rpi5CarButtonEventSequence.cpp)
target_link_libraries(xWalkButtonEventSequence PRIVATE xWalkTrace)
target_include_directories(xWalkButtonEventSequence PUBLIC core/include)
target_compile_features(xWalkButtonEventSequence PUBLIC cxx_std_17)
target_link_libraries(xWalkButtonEventSequence PUBLIC xWalkGpio)

add_library(xWalkInitAnglesSequence
    core/src/xHal_Rpi5CarInitAnglesSequence.cpp)
target_include_directories(xWalkInitAnglesSequence PUBLIC core/include)
target_compile_features(xWalkInitAnglesSequence PUBLIC cxx_std_17)
target_link_libraries(xWalkInitAnglesSequence PUBLIC
    xWalkBoardControl
    xWalkRobot)

add_library(xWalkRobotHat5MotorSequence
    core/src/xHal_Rpi5CarRobotHat5MotorSequence.cpp)
target_include_directories(xWalkRobotHat5MotorSequence PUBLIC core/include)
target_compile_features(xWalkRobotHat5MotorSequence PUBLIC cxx_std_17)
target_link_libraries(xWalkRobotHat5MotorSequence
    PUBLIC xWalkMotor
    PRIVATE xWalkTrace)

add_library(xWalkMotorSequence
    core/src/xHal_Rpi5CarMotorSequence.cpp)
target_include_directories(xWalkMotorSequence PUBLIC core/include)
target_compile_features(xWalkMotorSequence PUBLIC cxx_std_17)
target_link_libraries(xWalkMotorSequence
    PUBLIC xWalkMotor
    PRIVATE xWalkTrace)

add_library(xWalkServoHatSequence
    core/src/xHal_Rpi5CarServoHatSequence.cpp)
target_link_libraries(xWalkServoHatSequence PRIVATE xWalkTrace)
target_include_directories(xWalkServoHatSequence PUBLIC core/include)
target_compile_features(xWalkServoHatSequence PUBLIC cxx_std_17)
target_link_libraries(xWalkServoHatSequence PUBLIC
    xWalkAdc
    xWalkBoardControl
    xWalkServo)

add_library(xWalkServoSequence
    core/src/xHal_Rpi5CarServoSequence.cpp)
target_link_libraries(xWalkServoSequence PRIVATE xWalkTrace)
target_include_directories(xWalkServoSequence PUBLIC core/include)
target_compile_features(xWalkServoSequence PUBLIC cxx_std_17)
target_link_libraries(xWalkServoSequence PUBLIC xWalkServo)

add_library(xWalkPiperStreamSequence
    core/src/xHal_Rpi5CarPiperStreamSequence.cpp)
target_link_libraries(xWalkPiperStreamSequence PRIVATE xWalkTrace)
target_include_directories(xWalkPiperStreamSequence PUBLIC core/include)
target_compile_features(xWalkPiperStreamSequence PUBLIC cxx_std_17)
target_link_libraries(xWalkPiperStreamSequence PUBLIC xWalkLibraryCommon)

add_library(xWalkToneSequence
    core/src/xHal_Rpi5CarToneSequence.cpp)
target_link_libraries(xWalkToneSequence PRIVATE xWalkTrace)
target_include_directories(xWalkToneSequence PUBLIC core/include)
target_compile_features(xWalkToneSequence PUBLIC cxx_std_17)
target_link_libraries(xWalkToneSequence PUBLIC xWalkMusic)

if(XWALK_HAL_BUILD_RPI)
    find_package(yaml-cpp CONFIG REQUIRED)

    add_library(xWalkButtonEventSequenceLinux
        hardware/src/xHal_Rpi5CarButtonEventSequenceLinux.cpp)
    target_include_directories(xWalkButtonEventSequenceLinux PUBLIC hardware/include)
    target_compile_features(xWalkButtonEventSequenceLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkButtonEventSequenceLinux PUBLIC
        xWalkButtonEventSequence
        PRIVATE xWalkTrace)

    add_library(xWalkInitAnglesSequenceLinux
        hardware/src/xHal_Rpi5CarInitAnglesSequenceLinux.cpp)
    target_include_directories(xWalkInitAnglesSequenceLinux PUBLIC hardware/include)
    target_compile_features(xWalkInitAnglesSequenceLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkInitAnglesSequenceLinux PUBLIC
        xWalkInitAnglesSequence
        xWalkGpioLinux
        xWalkI2cLinux)

    add_library(xWalkRobotHat5MotorSequenceLinux
        hardware/src/xHal_Rpi5CarRobotHat5MotorSequenceLinux.cpp)
    target_include_directories(xWalkRobotHat5MotorSequenceLinux PUBLIC hardware/include)
    target_compile_features(xWalkRobotHat5MotorSequenceLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkRobotHat5MotorSequenceLinux PUBLIC
        xWalkI2cLinux
        xWalkRobotHat5MotorSequence)

    add_library(xWalkMotorSequenceLinux
        hardware/src/xHal_Rpi5CarMotorSequenceLinux.cpp)
    target_include_directories(xWalkMotorSequenceLinux PUBLIC hardware/include)
    target_compile_features(xWalkMotorSequenceLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkMotorSequenceLinux PUBLIC
        xWalkGpioLinux
        xWalkI2cLinux
        xWalkMotorSequence)

    add_library(xWalkServoHatSequenceLinux
        hardware/src/xHal_Rpi5CarServoHatSequenceLinux.cpp)
    target_include_directories(xWalkServoHatSequenceLinux PUBLIC hardware/include)
    target_compile_features(xWalkServoHatSequenceLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkServoHatSequenceLinux PUBLIC
        xWalkGpioLinux
        xWalkI2cLinux
        xWalkServoHatSequence
        PRIVATE xWalkTrace)

    add_library(xWalkServoSequenceLinux
        hardware/src/xHal_Rpi5CarServoSequenceLinux.cpp)
    target_include_directories(xWalkServoSequenceLinux PUBLIC hardware/include)
    target_compile_features(xWalkServoSequenceLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkServoSequenceLinux PUBLIC
        xWalkI2cLinux
        xWalkServoSequence)

    add_library(xWalkToneSequenceLinux
        hardware/src/xHal_Rpi5CarToneSequenceLinux.cpp)
    target_include_directories(xWalkToneSequenceLinux PUBLIC hardware/include)
    target_compile_features(xWalkToneSequenceLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkToneSequenceLinux PUBLIC
        xWalkMusicAlsa
        xWalkToneSequence
        PRIVATE xWalkTrace)

    add_library(xWalkSequenceTestRunner
        hardware/src/xHal_Rpi5CarSequenceTestRunner.cpp)
    target_include_directories(xWalkSequenceTestRunner PUBLIC hardware/include)
    target_compile_features(xWalkSequenceTestRunner PUBLIC cxx_std_17)
    target_compile_definitions(xWalkSequenceTestRunner PRIVATE
        XHAL_RPI5CAR_SEQUENCE_TEST_YAML_PATH="${XWALK_SEQUENCE_TEST_YAML_OUTPUT}")
    target_link_libraries(xWalkSequenceTestRunner
        PUBLIC xWalkLibraryCommon
        PRIVATE
        xWalkButtonEventSequenceLinux
        xWalkGpioLinux
        xWalkInitAnglesSequenceLinux
        xWalkMotorSequenceLinux
        xWalkRobotHat5MotorSequenceLinux
        xWalkServoHatSequenceLinux
        xWalkServoSequenceLinux
        xWalkToneSequenceLinux
        yaml-cpp::yaml-cpp)

    add_executable(xSequenceTest main.cpp)
    set_target_properties(xSequenceTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    target_compile_features(xSequenceTest PRIVATE cxx_std_17)
    target_link_libraries(xSequenceTest PRIVATE xWalkSequenceTestRunner)
    add_test(NAME xWalkButtonEventSequenceHardwareTest
        COMMAND xSequenceTest button-event)
    add_test(NAME xWalkInitAnglesSequenceHardwareTest
        COMMAND xSequenceTest init-angles)
    add_test(NAME xWalkRobotHat5MotorSequenceHardwareTest
        COMMAND xSequenceTest motor-robothat5)
    add_test(NAME xWalkMotorSequenceHardwareTest
        COMMAND xSequenceTest motor)
    add_test(NAME xWalkServoHatSequenceHardwareTest
        COMMAND xSequenceTest servo-hat)
    add_test(NAME xWalkServoSequenceHardwareTest
        COMMAND xSequenceTest servo)
    add_test(NAME xWalkToneSequenceHardwareTest
        COMMAND xSequenceTest tone)
    set_tests_properties(xWalkButtonEventSequenceHardwareTest PROPERTIES
        LABELS "hardware;sequence"
        TIMEOUT 45)
    set_tests_properties(xWalkInitAnglesSequenceHardwareTest PROPERTIES
        LABELS "hardware;sequence"
        TIMEOUT 30)
    set_tests_properties(xWalkRobotHat5MotorSequenceHardwareTest PROPERTIES
        LABELS "hardware;sequence"
        TIMEOUT 15)
    set_tests_properties(xWalkMotorSequenceHardwareTest PROPERTIES
        LABELS "hardware;sequence"
        TIMEOUT 15)
    set_tests_properties(xWalkServoHatSequenceHardwareTest PROPERTIES
        LABELS "hardware;sequence"
        TIMEOUT 30)
    set_tests_properties(xWalkServoSequenceHardwareTest PROPERTIES
        LABELS "hardware;sequence"
        TIMEOUT 15)
    set_tests_properties(xWalkToneSequenceHardwareTest PROPERTIES
        DISABLED TRUE
        LABELS "hardware;sequence"
        TIMEOUT 75)
endif()
