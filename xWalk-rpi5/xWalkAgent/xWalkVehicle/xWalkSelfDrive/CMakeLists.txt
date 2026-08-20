cmake_minimum_required(VERSION 3.16)

project(xWalkSelfDrive LANGUAGES CXX)

option(XWALK_SELF_DRIVE_BUILD_HOST_TESTS "Build xWalkSelfDrive host tests" OFF)
option(XWALK_SELF_DRIVE_BUILD_HARDWARE_TESTS "Compile xWalkSelfDrive with RPi dependencies" OFF)

if(XWALK_SELF_DRIVE_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkSelfDrive RPi mode requires Linux support")
endif()

set(XWALK_SELF_DRIVE_HAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal CACHE PATH
    "Path to the xWalk HAL source tree used by xWalkSelfDrive")

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL "Self-drive dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS ${XWALK_SELF_DRIVE_BUILD_HARDWARE_TESTS}
        CACHE BOOL "Self-drive RPi dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPicarx ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkMusic)
    set(XWALK_MUSIC_BUILD_HOST_TESTS OFF CACHE BOOL "Self-drive dependency setting" FORCE)
    add_subdirectory(${XWALK_SELF_DRIVE_HAL_ROOT}/layer1/xWalkMusic
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkMusic)
endif()

add_library(xWalkSelfDrive STATIC
    src/xAgent_Rpi5CarSelfDriveActions.cpp
    src/xAgent_Rpi5CarSelfDriveGestures.cpp
    src/xAgent_Rpi5CarSelfDriveLifecycle.cpp
    src/xAgent_Rpi5CarSelfDriveWorker.cpp
)
target_link_libraries(xWalkSelfDrive PRIVATE xWalkTrace)
add_library(xWalk::SelfDrive ALIAS xWalkSelfDrive)

target_include_directories(xWalkSelfDrive PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkSelfDrive PUBLIC cxx_std_17)
target_link_libraries(xWalkSelfDrive PUBLIC xWalkPicarx xWalkMusic)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkSelfDrive PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_SELF_DRIVE_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkSelfDriveTest
        test/src/xAgent_Rpi5CarSelfDriveTest.cpp
        test/src/xAgent_Rpi5CarSelfDriveTestSupport.cpp)
    target_include_directories(xWalkSelfDriveTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_compile_definitions(xWalkSelfDriveTest PRIVATE
        XWALK_TEST_SOUND_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkAudioResources/sounds"
        XWALK_TEST_MUSIC_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkAudioResources/music")
    target_link_libraries(xWalkSelfDriveTest PRIVATE xWalkSelfDrive)
    add_test(NAME xWalkSelfDriveHostTest COMMAND xWalkSelfDriveTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/self-drive-test.conf)
    set_tests_properties(xWalkSelfDriveHostTest PROPERTIES LABELS "host")
endif()
