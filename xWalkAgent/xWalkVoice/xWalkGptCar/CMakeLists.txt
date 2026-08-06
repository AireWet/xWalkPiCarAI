cmake_minimum_required(VERSION 3.16)
project(xWalkGptCar LANGUAGES CXX)

option(XWALK_GPT_CAR_BUILD_HOST_TESTS "Build GPT-car host tests" OFF)

if(NOT TARGET xWalkVoiceActiveCar)
    set(XWALK_VOICE_ACTIVE_CAR_BUILD_HOST_TESTS OFF CACHE BOOL "GPT-car dependency" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkVoiceActiveCar
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceActiveCar)
endif()

add_library(xWalkGptCar STATIC src/xAgent_Rpi5CarGptCar.cpp)
add_library(xWalk::GptCar ALIAS xWalkGptCar)
target_include_directories(xWalkGptCar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkGptCar PUBLIC cxx_std_17)
target_link_libraries(xWalkGptCar PUBLIC xWalkVoiceActiveCar)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkGptCar PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_GPT_CAR_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkGptCarTest test/src/xAgent_Rpi5CarGptCarTest.cpp)
    target_link_libraries(xWalkGptCarTest PRIVATE xWalkGptCar)
    add_test(NAME xWalkGptCarHostTest COMMAND xWalkGptCarTest)
    set_tests_properties(xWalkGptCarHostTest PROPERTIES LABELS "host")
endif()
