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

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkPwm)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../device/xWalkPwm ${CMAKE_CURRENT_BINARY_DIR}/xWalkPwm)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkBuzzer STATIC
    src/xHal_Rpi5CarBuzzer.cpp
    src/xHal_Rpi5CarBuzzerLifecycle.cpp
)

target_include_directories(xWalkBuzzer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkBuzzer PUBLIC cxx_std_17)
target_link_libraries(xWalkBuzzer PUBLIC xWalkLibraryCommon xWalkPwm xWalkGpio PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkBuzzer PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_BUZZER_BUILD_HOST_TESTS OR XWALK_BUZZER_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_BUZZER_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_BUZZER_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_BUZZER_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarBuzzerTraceConfig.py)
    set(XWALK_BUZZER_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkBuzzerTrace.xml)
    add_custom_command(OUTPUT ${XWALK_BUZZER_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_BUZZER_TRACE_GENERATOR}
            --input ${XWALK_BUZZER_TRACE_INVENTORY}
            --output ${XWALK_BUZZER_TRACE_CONFIG}
        DEPENDS ${XWALK_BUZZER_TRACE_INVENTORY} ${XWALK_BUZZER_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkBuzzer trace catalogue" VERBATIM)
    add_custom_target(xWalkBuzzerTestTraceConfig ALL DEPENDS ${XWALK_BUZZER_TRACE_CONFIG})
    add_dependencies(xWalkBuzzerTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkBuzzerTest
        test/src/xHal_Rpi5CarBuzzerTest.cpp
        test/src/xHal_Rpi5CarBuzzerTestSupport.cpp
        simulation/src/xHal_Rpi5CarBuzzerSimulationArguments.cpp)
    target_include_directories(xWalkBuzzerTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_BUZZER_TEST_TRACE_LOG
        ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkBuzzerTest.log)
    target_compile_definitions(xWalkBuzzerTest PRIVATE
        XWALK_BUZZER_SIMULATION_TRACE_CONFIG_PATH="${XWALK_BUZZER_TRACE_CONFIG}"
        XWALK_BUZZER_SIMULATION_TRACE_LOG_PATH="${XWALK_BUZZER_TEST_TRACE_LOG}")
    target_link_libraries(xWalkBuzzerTest PRIVATE xWalkBuzzer xWalkTrace)
    add_dependencies(xWalkBuzzerTest xWalkBuzzerTestTraceConfig)
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
