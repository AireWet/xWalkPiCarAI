cmake_minimum_required(VERSION 3.16)
project(xWalkVoiceActiveCarGpt LANGUAGES CXX)

option(XWALK_VOICE_ACTIVE_CAR_GPT_BUILD_HOST_TESTS "Build GPT voice-car host tests" OFF)

if(NOT TARGET xWalkVoiceActiveCar)
    set(XWALK_VOICE_ACTIVE_CAR_BUILD_HOST_TESTS OFF CACHE BOOL "GPT voice car dependency" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkVoiceActiveCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceActiveCar)
endif()

add_library(xWalkVoiceActiveCarGpt STATIC src/xAgent_Rpi5CarVoiceActiveCarGpt.cpp)
add_library(xWalk::VoiceActiveCarGpt ALIAS xWalkVoiceActiveCarGpt)
target_include_directories(xWalkVoiceActiveCarGpt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVoiceActiveCarGpt PUBLIC cxx_std_17)
target_link_libraries(xWalkVoiceActiveCarGpt PUBLIC xWalkVoiceActiveCar)
target_link_libraries(xWalkVoiceActiveCarGpt PRIVATE xWalkTrace)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkVoiceActiveCarGpt PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_VOICE_ACTIVE_CAR_GPT_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkVoiceActiveCarGptTest
        test/src/xAgent_Rpi5CarVoiceActiveCarGptTest.cpp)
    target_link_libraries(xWalkVoiceActiveCarGptTest PRIVATE xWalkVoiceActiveCarGpt)
    add_test(NAME xWalkVoiceActiveCarGptHostTest COMMAND xWalkVoiceActiveCarGptTest)
    set_tests_properties(xWalkVoiceActiveCarGptHostTest PROPERTIES LABELS "host")
endif()
