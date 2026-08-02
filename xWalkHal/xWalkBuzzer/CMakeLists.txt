cmake_minimum_required(VERSION 3.16)
project(xWalkBuzzer LANGUAGES CXX)

option(XWALK_BUZZER_BUILD_HOST_TESTS "Build xWalkBuzzer host tests" OFF)
option(XWALK_BUZZER_BUILD_HARDWARE_TESTS "Build xWalkBuzzer hardware tests" OFF)

set(XWALK_PWM_BUILD_HOST_TESTS ${XWALK_BUZZER_BUILD_HOST_TESTS})
set(XWALK_PWM_BUILD_HARDWARE_TESTS ${XWALK_BUZZER_BUILD_HARDWARE_TESTS})
set(XWALK_GPIO_BUILD_HOST_TESTS ${XWALK_BUZZER_BUILD_HOST_TESTS})
set(XWALK_GPIO_BUILD_HARDWARE_TESTS ${XWALK_BUZZER_BUILD_HARDWARE_TESTS})

if(XWALK_BUZZER_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkBuzzer hardware tests require Linux device APIs")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

if(NOT TARGET xWalkPwm)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPwm ${CMAKE_CURRENT_BINARY_DIR}/xWalkPwm)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

add_library(xWalkBuzzer STATIC
    src/xHal_Rpi5CarBuzzer.cpp
    src/xHal_Rpi5CarBuzzerLifecycle.cpp
)

target_include_directories(xWalkBuzzer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkBuzzer PUBLIC cxx_std_17)
target_link_libraries(xWalkBuzzer PUBLIC xWalkCommon xWalkPwm xWalkGpio)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkBuzzer PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_BUZZER_BUILD_HOST_TESTS OR XWALK_BUZZER_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_BUZZER_BUILD_HOST_TESTS)
    add_executable(xWalkBuzzerTest test/src/xHal_Rpi5CarBuzzerTest.cpp)
    target_link_libraries(xWalkBuzzerTest PRIVATE xWalkBuzzer)
    add_test(NAME xWalkBuzzerHostTest COMMAND xWalkBuzzerTest)
    set_tests_properties(xWalkBuzzerHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_BUZZER_BUILD_HARDWARE_TESTS)
    add_executable(xWalkBuzzerHardwareTest
        test/hardware/src/xHal_Rpi5CarBuzzerHardwareTest.cpp)
    target_link_libraries(xWalkBuzzerHardwareTest PRIVATE xWalkBuzzer xWalkI2cLinux)
    add_test(NAME xWalkBuzzerHardwareInactiveTest COMMAND xWalkBuzzerHardwareTest)
    set_tests_properties(xWalkBuzzerHardwareInactiveTest PROPERTIES LABELS "hardware")
endif()
