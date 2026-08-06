cmake_minimum_required(VERSION 3.16)

project(xCliGoogleTest LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/XWalkDependencies.cmake)

find_package(GTest CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)

if(NOT TARGET xWalkController)
    message(FATAL_ERROR "xCliGoogleTest requires the xWalkController target")
endif()

set(XWALK_CLI_GOOGLE_TEST_CONFIG_OUTPUT
    ${CMAKE_BINARY_DIR}/xCliGoogleTestConfig.xml)
configure_file(config/test_config.xml
    ${XWALK_CLI_GOOGLE_TEST_CONFIG_OUTPUT} COPYONLY)

add_library(xCliGoogleControllerTest OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkHandler/test/src/xControllerTest.cpp)
target_compile_features(xCliGoogleControllerTest PRIVATE cxx_std_17)
target_compile_definitions(xCliGoogleControllerTest PRIVATE
    main=xWalkControllerLegacyMain
    XWALK_TEST_SOUND_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkAudioResources/sounds")
target_link_libraries(xCliGoogleControllerTest PRIVATE xWalkController)

add_executable(xCliGoogleTest
    src/main.cpp
    src/xCliTestConfig.cpp
    $<TARGET_OBJECTS:xCliGoogleControllerTest>)
set_target_properties(xCliGoogleTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
target_include_directories(xCliGoogleTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xCliGoogleTest PRIVATE cxx_std_17)
target_link_libraries(xCliGoogleTest PRIVATE
    GTest::gtest
    tinyxml2::tinyxml2
    xWalkController)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xCliGoogleTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_test(NAME xCliGoogleTestHostTest COMMAND xCliGoogleTest)
set_tests_properties(xCliGoogleTestHostTest PROPERTIES LABELS "host;unit;cli")
