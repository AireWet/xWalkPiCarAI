cmake_minimum_required(VERSION 3.16)

project(xWalkGrayscaleCalibration LANGUAGES CXX)

option(XWALK_GRAYSCALE_CALIBRATION_BUILD_HOST_TESTS
    "Build xWalkGrayscaleCalibration host tests" OFF)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Grayscale-calibration dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Grayscale-calibration dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

add_library(xWalkGrayscaleCalibration STATIC
    src/xAgent_Rpi5CarGrayscaleCalibration.cpp
    src/xAgent_Rpi5CarGrayscaleCalibrationLifecycle.cpp)
target_link_libraries(xWalkGrayscaleCalibration PRIVATE xWalkTrace)
add_library(xWalk::GrayscaleCalibration ALIAS xWalkGrayscaleCalibration)

target_include_directories(xWalkGrayscaleCalibration PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkGrayscaleCalibration PUBLIC cxx_std_17)
target_link_libraries(xWalkGrayscaleCalibration PUBLIC xWalkPicarx)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkGrayscaleCalibration PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_GRAYSCALE_CALIBRATION_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkGrayscaleCalibrationTest
        test/src/xAgent_Rpi5CarGrayscaleCalibrationTest.cpp)
    target_link_libraries(xWalkGrayscaleCalibrationTest PRIVATE
        xWalkGrayscaleCalibration)
    add_test(NAME xWalkGrayscaleCalibrationHostTest
        COMMAND xWalkGrayscaleCalibrationTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/grayscale-calibration-test.conf)
    set_tests_properties(xWalkGrayscaleCalibrationHostTest PROPERTIES LABELS "host")
endif()
