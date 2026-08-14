cmake_minimum_required(VERSION 3.16)

project(xWalkObstacleAvoidance LANGUAGES CXX)

option(XWALK_OBSTACLE_AVOIDANCE_BUILD_HOST_TESTS
    "Build xWalkObstacleAvoidance host tests" OFF)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Obstacle-avoidance dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Obstacle-avoidance dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

add_library(xWalkObstacleAvoidance STATIC
    src/xAgent_Rpi5CarObstacleAvoidance.cpp
    src/xAgent_Rpi5CarObstacleAvoidanceLifecycle.cpp)
target_link_libraries(xWalkObstacleAvoidance PRIVATE xWalkTrace)
add_library(xWalk::ObstacleAvoidance ALIAS xWalkObstacleAvoidance)

target_include_directories(xWalkObstacleAvoidance PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkObstacleAvoidance PUBLIC cxx_std_17)
target_link_libraries(xWalkObstacleAvoidance PUBLIC xWalkPicarx)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkObstacleAvoidance PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_OBSTACLE_AVOIDANCE_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkObstacleAvoidanceTest
        test/src/xAgent_Rpi5CarObstacleAvoidanceTest.cpp)
    target_link_libraries(xWalkObstacleAvoidanceTest PRIVATE xWalkObstacleAvoidance)
    add_test(NAME xWalkObstacleAvoidanceHostTest COMMAND xWalkObstacleAvoidanceTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/obstacle-avoidance-test.conf)
    set_tests_properties(xWalkObstacleAvoidanceHostTest PROPERTIES LABELS "host")
endif()
