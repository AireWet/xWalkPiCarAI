cmake_minimum_required(VERSION 3.16)
project(xWalkI2cSimulation LANGUAGES CXX)

set(XWALK_I2C_SIMULATION_BACKEND "stub" CACHE STRING
    "Simulation device backend: stub or hardware")
set_property(CACHE XWALK_I2C_SIMULATION_BACKEND PROPERTY STRINGS stub hardware)
set(XWALK_I2C_SIMULATION_DEVICE "/dev/i2c-1" CACHE FILEPATH
    "Linux I2C device used by the hardware backend")

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkI2c simulation requires Linux i2c-dev")
endif()
if(NOT XWALK_I2C_SIMULATION_BACKEND STREQUAL "stub"
    AND NOT XWALK_I2C_SIMULATION_BACKEND STREQUAL "hardware")
    message(FATAL_ERROR "XWALK_I2C_SIMULATION_BACKEND must be stub or hardware")
endif()

set(XWALK_I2C_BUILD_LINUX_BACKEND ON CACHE BOOL
    "Required by the standalone simulation" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/xWalkI2c)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_I2C_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_I2C_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarI2cTraceConfig.py)
set(XWALK_I2C_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkI2cTrace.xml)

add_custom_command(
    OUTPUT ${XWALK_I2C_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_I2C_TRACE_GENERATOR}
        --input ${XWALK_I2C_TRACE_INVENTORY}
        --output ${XWALK_I2C_TRACE_CONFIG}
    DEPENDS ${XWALK_I2C_TRACE_INVENTORY} ${XWALK_I2C_TRACE_GENERATOR}
    COMMENT "Preparing the all-disabled xWalkI2c trace catalogue"
    VERBATIM)
add_custom_target(xWalkI2cSimulationTraceConfig ALL
    DEPENDS ${XWALK_I2C_TRACE_CONFIG})
add_dependencies(xWalkI2cSimulationTraceConfig xWalkTraceMetadata)

set(XWALK_I2C_SIMULATION_SOURCES
    src/main.cpp
    src/xHal_Rpi5CarI2cHandler.cpp
    src/xHal_Rpi5CarI2cSimulationArguments.cpp)
if(XWALK_I2C_SIMULATION_BACKEND STREQUAL "stub")
    list(APPEND XWALK_I2C_SIMULATION_SOURCES
        src/xHal_Rpi5CarI2cDeviceFactoryStub.cpp
        src/xHal_Rpi5CarI2cHostStub.cpp)
else()
    list(APPEND XWALK_I2C_SIMULATION_SOURCES
        src/xHal_Rpi5CarI2cDeviceFactoryHardware.cpp)
endif()

add_executable(xWalkI2cSimulation ${XWALK_I2C_SIMULATION_SOURCES})
target_include_directories(xWalkI2cSimulation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkI2cSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkI2cSimulation PRIVATE
    XWALK_I2C_SIMULATION_DEVICE_PATH="${XWALK_I2C_SIMULATION_DEVICE}"
    XWALK_I2C_SIMULATION_TRACE_CONFIG_PATH="${XWALK_I2C_TRACE_CONFIG}"
    XWALK_I2C_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkI2cSimulation.log")
target_link_libraries(xWalkI2cSimulation PRIVATE xWalkI2cLinux xWalkTrace)
add_dependencies(xWalkI2cSimulation xWalkI2cSimulationTraceConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkI2cSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
