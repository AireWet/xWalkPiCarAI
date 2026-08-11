cmake_minimum_required(VERSION 3.16)

project(xWalkRoadUserSafety LANGUAGES CXX)

option(XWALK_ROAD_USER_SAFETY_BUILD_HOST_TESTS
    "Build deterministic road-user safety host tests" OFF)

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

add_library(xWalkRoadUserSafety STATIC
    src/xAgent_Rpi5CarRoadUserSafety.cpp)
target_link_libraries(xWalkRoadUserSafety PRIVATE xWalkTrace)
add_library(xWalk::RoadUserSafety ALIAS xWalkRoadUserSafety)
target_include_directories(xWalkRoadUserSafety PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkRoadUserSafety PUBLIC cxx_std_17)
target_link_libraries(xWalkRoadUserSafety PUBLIC xWalkLibraryCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkRoadUserSafety PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_ROAD_USER_SAFETY_BUILD_HOST_TESTS)
    include(CTest)
    find_package(GTest CONFIG REQUIRED)
    find_package(OpenCV 4 REQUIRED COMPONENTS core imgproc videoio)
    add_executable(xWalkRoadUserSafetyTest
        test/src/xAgent_Rpi5CarRoadUserSafetyTest.cpp
        test/src/xAgent_Rpi5CarRoadUserSafetyAssetTest.cpp
        test/src/xAgent_Rpi5CarRoadUserSafetyRecordedTest.cpp
        test/src/xAgent_Rpi5CarRoadUserSafetyTestSupport.cpp)
    target_include_directories(xWalkRoadUserSafetyTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_link_libraries(xWalkRoadUserSafetyTest PRIVATE
        xWalkRoadUserSafety GTest::gtest_main
        opencv_core opencv_imgproc opencv_videoio)
    target_compile_definitions(xWalkRoadUserSafetyTest PRIVATE
        XWALK_RECORDED_ASSET_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../test/assets")
    add_test(NAME xWalkRoadUserSafetyHostTest COMMAND xWalkRoadUserSafetyTest)
    set_tests_properties(xWalkRoadUserSafetyHostTest PROPERTIES
        LABELS "host;vision;risk;fault-injection;recorded-media;recorded-scenario")
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME xWalkVisionAssetManifestTest
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/../test/assets/validate-assets.py
            ${CMAKE_CURRENT_SOURCE_DIR}/../test/assets)
    set_tests_properties(xWalkVisionAssetManifestTest PROPERTIES
        LABELS "host;vision;recorded-media;fixture-validation")
endif()
