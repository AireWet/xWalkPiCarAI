cmake_minimum_required(VERSION 3.16)

project(xWalkServoMotorCalibration LANGUAGES CXX)

option(XWALK_SERVO_MOTOR_CALIBRATION_BUILD_HOST_TESTS
    "Build xWalkServoMotorCalibration host tests" OFF)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Servo/motor-calibration dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Servo/motor-calibration dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

add_library(xWalkServoMotorCalibration STATIC
    src/xAgent_Rpi5CarServoMotorCalibration.cpp
    src/xAgent_Rpi5CarServoMotorCalibrationLifecycle.cpp)
add_library(xWalk::ServoMotorCalibration ALIAS xWalkServoMotorCalibration)

target_include_directories(xWalkServoMotorCalibration PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkServoMotorCalibration PUBLIC cxx_std_17)
target_link_libraries(xWalkServoMotorCalibration PUBLIC xWalkPicarx)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkServoMotorCalibration PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_SERVO_MOTOR_CALIBRATION_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkServoMotorCalibrationTest
        test/src/xAgent_Rpi5CarServoMotorCalibrationTest.cpp)
    target_link_libraries(xWalkServoMotorCalibrationTest PRIVATE
        xWalkServoMotorCalibration)
    add_test(NAME xWalkServoMotorCalibrationHostTest
        COMMAND xWalkServoMotorCalibrationTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/servo-motor-calibration-test.conf)
    set_tests_properties(xWalkServoMotorCalibrationHostTest PROPERTIES LABELS "host")
endif()
