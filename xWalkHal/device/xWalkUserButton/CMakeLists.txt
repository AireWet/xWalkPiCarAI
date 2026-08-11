cmake_minimum_required(VERSION 3.16)
project(xWalkUserButton LANGUAGES CXX)

option(XWALK_USER_BUTTON_BUILD_HOST_TESTS "Build xWalkUserButton host tests" OFF)
option(XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS "Build xWalkUserButton hardware tests" OFF)

set(XWALK_GPIO_BUILD_HOST_TESTS ${XWALK_USER_BUTTON_BUILD_HOST_TESTS})
set(XWALK_GPIO_BUILD_HARDWARE_TESTS ${XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS})

if(XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkUserButton hardware tests require the Linux GPIO API")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkUserButton STATIC
    src/xHal_Rpi5CarUserButton.cpp
    src/xHal_Rpi5CarUserButtonLifecycle.cpp
)

target_include_directories(xWalkUserButton PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkUserButton PUBLIC cxx_std_17)
target_link_libraries(xWalkUserButton PUBLIC xWalkLibraryCommon xWalkGpio PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkUserButton PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_USER_BUTTON_BUILD_HOST_TESTS OR XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_USER_BUTTON_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_USER_BUTTON_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_USER_BUTTON_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarUserButtonTraceConfig.py)
    set(XWALK_USER_BUTTON_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkUserButtonTrace.xml)
    add_custom_command(OUTPUT ${XWALK_USER_BUTTON_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_USER_BUTTON_TRACE_GENERATOR}
            --input ${XWALK_USER_BUTTON_TRACE_INVENTORY}
            --output ${XWALK_USER_BUTTON_TRACE_CONFIG}
        DEPENDS ${XWALK_USER_BUTTON_TRACE_INVENTORY} ${XWALK_USER_BUTTON_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkUserButton trace catalogue" VERBATIM)
    add_custom_target(xWalkUserButtonTestTraceConfig ALL
        DEPENDS ${XWALK_USER_BUTTON_TRACE_CONFIG})
    add_dependencies(xWalkUserButtonTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkUserButtonTest
        test/src/xHal_Rpi5CarUserButtonTest.cpp
        test/src/xHal_Rpi5CarUserButtonTestSupport.cpp
        simulation/src/xHal_Rpi5CarUserButtonSimulationArguments.cpp)
    target_include_directories(xWalkUserButtonTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_USER_BUTTON_TEST_TRACE_LOG
        ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkUserButtonTest.log)
    target_compile_definitions(xWalkUserButtonTest PRIVATE
        XWALK_USER_BUTTON_SIMULATION_TRACE_CONFIG_PATH="${XWALK_USER_BUTTON_TRACE_CONFIG}"
        XWALK_USER_BUTTON_SIMULATION_TRACE_LOG_PATH="${XWALK_USER_BUTTON_TEST_TRACE_LOG}")
    target_link_libraries(xWalkUserButtonTest PRIVATE xWalkUserButton xWalkTrace)
    add_dependencies(xWalkUserButtonTest xWalkUserButtonTestTraceConfig)
    add_test(NAME xWalkUserButtonHostTest COMMAND xWalkUserButtonTest)
    set_tests_properties(xWalkUserButtonHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS)
    add_executable(xWalkUserButtonHardwareTest
        test/hardware/src/xHal_Rpi5CarUserButtonHardwareTest.cpp)
    target_link_libraries(xWalkUserButtonHardwareTest PRIVATE xWalkUserButton xWalkGpioLinux)
    add_test(NAME xWalkUserButtonHardwareMonitorTest COMMAND xWalkUserButtonHardwareTest)
    set_tests_properties(xWalkUserButtonHardwareMonitorTest PROPERTIES LABELS "hardware")
endif()
