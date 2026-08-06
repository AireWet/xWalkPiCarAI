cmake_minimum_required(VERSION 3.16)

project(xWalkComputerVision LANGUAGES CXX)

option(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS
    "Build xWalkComputerVision host tests" OFF)
option(XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND
    "Build the OpenCV camera provider" OFF)
option(XWALK_COMPUTER_VISION_BUILD_HARDWARE_TESTS
    "Build the opt-in physical-camera test" OFF)

if(XWALK_COMPUTER_VISION_BUILD_HARDWARE_TESTS AND
    NOT XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND)
    message(FATAL_ERROR
        "Computer-vision hardware tests require the OpenCV backend")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

add_library(xWalkComputerVision STATIC
    src/xAgent_Rpi5CarComputerVision.cpp
    src/xAgent_Rpi5CarComputerVisionLifecycle.cpp)
add_library(xWalk::ComputerVision ALIAS xWalkComputerVision)

target_include_directories(xWalkComputerVision PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkComputerVision PUBLIC cxx_std_17)
target_link_libraries(xWalkComputerVision PUBLIC xWalkLibraryCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkComputerVision PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND)
    find_package(OpenCV 4 REQUIRED COMPONENTS core imgcodecs imgproc objdetect videoio)
    add_library(xWalkComputerVisionOpenCv STATIC
        hardware/src/xAgent_Rpi5CarComputerVisionOpenCv.cpp)
    add_library(xWalk::ComputerVisionOpenCv ALIAS xWalkComputerVisionOpenCv)
    target_include_directories(xWalkComputerVisionOpenCv
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_include_directories(xWalkComputerVisionOpenCv SYSTEM PUBLIC
        ${OpenCV_INCLUDE_DIRS})
    target_compile_features(xWalkComputerVisionOpenCv PUBLIC cxx_std_17)
    target_link_libraries(xWalkComputerVisionOpenCv
        PUBLIC xWalkComputerVision
        PRIVATE ${OpenCV_LIBS})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkComputerVisionOpenCv PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkComputerVisionTest
        test/src/xAgent_Rpi5CarComputerVisionTest.cpp)
    target_link_libraries(xWalkComputerVisionTest PRIVATE xWalkComputerVision)
    add_test(NAME xWalkComputerVisionHostTest COMMAND xWalkComputerVisionTest)
    set_tests_properties(xWalkComputerVisionHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_COMPUTER_VISION_BUILD_HARDWARE_TESTS)
    include(CTest)
    add_executable(xWalkComputerVisionHardwareTest
        hardware/test/src/xAgent_Rpi5CarComputerVisionHardwareTest.cpp)
    target_link_libraries(xWalkComputerVisionHardwareTest PRIVATE
        xWalkComputerVisionOpenCv)
    add_test(NAME xWalkComputerVisionHardwareTest
        COMMAND xWalkComputerVisionHardwareTest
            /dev/video0
            /usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml
            /tmp/xwalk-computer-vision-hardware-test)
    set_tests_properties(xWalkComputerVisionHardwareTest PROPERTIES
        LABELS "hardware;camera")
endif()
