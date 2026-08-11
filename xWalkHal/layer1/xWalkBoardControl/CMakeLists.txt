cmake_minimum_required(VERSION 3.16)
project(xWalkBoardControl LANGUAGES CXX)

option(XWALK_BOARD_CONTROL_BUILD_HOST_TESTS "Build xWalkBoardControl host tests" OFF)
option(XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS "Build xWalkBoardControl hardware tests" OFF)

set(XWALK_ADC_BUILD_HOST_TESTS ${XWALK_BOARD_CONTROL_BUILD_HOST_TESTS})
set(XWALK_ADC_BUILD_HARDWARE_TESTS ${XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS})
set(XWALK_GPIO_BUILD_HOST_TESTS ${XWALK_BOARD_CONTROL_BUILD_HOST_TESTS})
set(XWALK_GPIO_BUILD_HARDWARE_TESTS ${XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS})
set(XWALK_I2C_BUILD_HOST_TESTS ${XWALK_BOARD_CONTROL_BUILD_HOST_TESTS})
set(XWALK_I2C_BUILD_HARDWARE_TESTS ${XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS})

if(XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkBoardControl hardware tests require Linux GPIO and I2C APIs")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

if(NOT TARGET xWalkAdc)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../device/xWalkAdc ${CMAKE_CURRENT_BINARY_DIR}/xWalkAdc)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

if(NOT TARGET xWalkI2c)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkI2c ${CMAKE_CURRENT_BINARY_DIR}/xWalkI2c)
endif()

add_library(xWalkBoardControl STATIC
    src/xHal_Rpi5CarBoardControl.cpp
    src/xHal_Rpi5CarBoardControlLifecycle.cpp
    src/xHal_Rpi5CarDevice.cpp
    src/xHal_Rpi5CarDeviceLifecycle.cpp
    src/xHal_Rpi5CarFirmwareInfo.cpp
    src/xHal_Rpi5CarFirmwareInfoLifecycle.cpp
)

target_include_directories(xWalkBoardControl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkBoardControl PUBLIC cxx_std_17)
target_link_libraries(xWalkBoardControl PUBLIC xWalkAdc xWalkLibraryCommon xWalkGpio xWalkI2c
    PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkBoardControl PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_BOARD_CONTROL_BUILD_HOST_TESTS OR XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_BOARD_CONTROL_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_BOARD_CONTROL_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_BOARD_CONTROL_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarBoardControlTraceConfig.py)
    set(XWALK_BOARD_CONTROL_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkBoardControlTrace.xml)
    add_custom_command(OUTPUT ${XWALK_BOARD_CONTROL_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_BOARD_CONTROL_TRACE_GENERATOR}
            --input ${XWALK_BOARD_CONTROL_TRACE_INVENTORY}
            --output ${XWALK_BOARD_CONTROL_TRACE_CONFIG}
        DEPENDS ${XWALK_BOARD_CONTROL_TRACE_INVENTORY} ${XWALK_BOARD_CONTROL_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkBoardControl trace catalogue" VERBATIM)
    add_custom_target(xWalkBoardControlTestTraceConfig ALL
        DEPENDS ${XWALK_BOARD_CONTROL_TRACE_CONFIG})
    add_dependencies(xWalkBoardControlTestTraceConfig xWalkTraceMetadata)
    set(XWALK_BOARD_CONTROL_TEST_SUPPORT
        test/src/xHal_Rpi5CarBoardControlTestSupport.cpp)
    set(XWALK_BOARD_CONTROL_TEST_DEFINITIONS
        XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH="${XWALK_BOARD_CONTROL_TRACE_CONFIG}"
        XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkBoardControlTest.log")
    add_executable(xWalkBoardControlTest test/src/xHal_Rpi5CarBoardControlTest.cpp)
    target_sources(xWalkBoardControlTest PRIVATE ${XWALK_BOARD_CONTROL_TEST_SUPPORT}
        simulation/src/xHal_Rpi5CarBoardControlSimulationArguments.cpp)
    target_include_directories(xWalkBoardControlTest PRIVATE test/include simulation/include)
    target_compile_definitions(xWalkBoardControlTest PRIVATE ${XWALK_BOARD_CONTROL_TEST_DEFINITIONS})
    target_link_libraries(xWalkBoardControlTest PRIVATE xWalkBoardControl xWalkTrace)
    add_dependencies(xWalkBoardControlTest xWalkBoardControlTestTraceConfig)
    add_test(NAME xWalkBoardControlHostTest COMMAND xWalkBoardControlTest)
    set_tests_properties(xWalkBoardControlHostTest PROPERTIES LABELS "host")

    add_executable(xWalkDeviceTest test/src/xHal_Rpi5CarDeviceTest.cpp)
    target_sources(xWalkDeviceTest PRIVATE ${XWALK_BOARD_CONTROL_TEST_SUPPORT})
    target_include_directories(xWalkDeviceTest PRIVATE test/include)
    target_compile_definitions(xWalkDeviceTest PRIVATE ${XWALK_BOARD_CONTROL_TEST_DEFINITIONS})
    target_link_libraries(xWalkDeviceTest PRIVATE xWalkBoardControl xWalkTrace)
    add_dependencies(xWalkDeviceTest xWalkBoardControlTestTraceConfig)
    add_test(NAME xWalkDeviceHostTest COMMAND xWalkDeviceTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-device-tree)
    set_tests_properties(xWalkDeviceHostTest PROPERTIES LABELS "host")

    add_executable(xWalkFirmwareInfoTest test/src/xHal_Rpi5CarFirmwareInfoTest.cpp)
    target_sources(xWalkFirmwareInfoTest PRIVATE ${XWALK_BOARD_CONTROL_TEST_SUPPORT})
    target_include_directories(xWalkFirmwareInfoTest PRIVATE test/include)
    target_compile_definitions(xWalkFirmwareInfoTest PRIVATE ${XWALK_BOARD_CONTROL_TEST_DEFINITIONS})
    target_link_libraries(xWalkFirmwareInfoTest PRIVATE xWalkBoardControl xWalkTrace)
    add_dependencies(xWalkFirmwareInfoTest xWalkBoardControlTestTraceConfig)
    add_test(NAME xWalkFirmwareInfoHostTest COMMAND xWalkFirmwareInfoTest)
    set_tests_properties(xWalkFirmwareInfoHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS)
    add_executable(xWalkBoardControlHardwareTest
        test/hardware/src/xHal_Rpi5CarBoardControlHardwareTest.cpp)
    target_link_libraries(xWalkBoardControlHardwareTest PRIVATE
        xWalkBoardControl xWalkGpioLinux xWalkI2cLinux)
    add_test(NAME xWalkBoardControlHardwareDisableTest COMMAND xWalkBoardControlHardwareTest)
    set_tests_properties(xWalkBoardControlHardwareDisableTest PROPERTIES LABELS "hardware")

    add_executable(xWalkDeviceHardwareTest
        test/hardware/src/xHal_Rpi5CarDeviceHardwareTest.cpp)
    target_link_libraries(xWalkDeviceHardwareTest PRIVATE xWalkBoardControl)
    add_test(NAME xWalkDeviceHardwareDiscoveryTest COMMAND xWalkDeviceHardwareTest)
    set_tests_properties(xWalkDeviceHardwareDiscoveryTest PROPERTIES LABELS "hardware")

    add_executable(xWalkFirmwareInfoHardwareTest
        test/hardware/src/xHal_Rpi5CarFirmwareInfoHardwareTest.cpp)
    target_link_libraries(xWalkFirmwareInfoHardwareTest PRIVATE
        xWalkBoardControl xWalkI2cLinux)
    add_test(NAME xWalkFirmwareInfoHardwareReadTest COMMAND xWalkFirmwareInfoHardwareTest)
    set_tests_properties(xWalkFirmwareInfoHardwareReadTest PROPERTIES LABELS "hardware")
endif()
