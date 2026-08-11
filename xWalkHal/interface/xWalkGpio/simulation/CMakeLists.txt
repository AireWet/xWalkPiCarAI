cmake_minimum_required(VERSION 3.16)
project(xWalkGpioSimulation LANGUAGES CXX)

set(XWALK_GPIO_SIMULATION_BACKEND "stub" CACHE STRING
    "Simulation device backend: stub or hardware")
set_property(CACHE XWALK_GPIO_SIMULATION_BACKEND PROPERTY STRINGS stub hardware)
set(XWALK_GPIO_SIMULATION_DEVICE "/dev/gpiochip0" CACHE FILEPATH
    "Linux GPIO device used by the hardware backend")

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkGpio simulation requires the Linux GPIO character-device API")
endif()
if(NOT XWALK_GPIO_SIMULATION_BACKEND STREQUAL "stub"
    AND NOT XWALK_GPIO_SIMULATION_BACKEND STREQUAL "hardware")
    message(FATAL_ERROR "XWALK_GPIO_SIMULATION_BACKEND must be stub or hardware")
endif()

set(XWALK_GPIO_BUILD_LINUX_BACKEND ON CACHE BOOL
    "Required by the standalone simulation" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_GPIO_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GPIO_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarGpioTraceConfig.py)
set(XWALK_GPIO_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkGpioTrace.xml)

add_custom_command(
    OUTPUT ${XWALK_GPIO_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GPIO_TRACE_GENERATOR}
        --input ${XWALK_GPIO_TRACE_INVENTORY}
        --output ${XWALK_GPIO_TRACE_CONFIG}
    DEPENDS ${XWALK_GPIO_TRACE_INVENTORY} ${XWALK_GPIO_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkGpio trace catalogue"
    VERBATIM)
add_custom_target(xWalkGpioSimulationTraceConfig ALL DEPENDS ${XWALK_GPIO_TRACE_CONFIG})
add_dependencies(xWalkGpioSimulationTraceConfig xWalkTraceMetadata)

set(XWALK_GPIO_SIMULATION_SOURCES
    src/main.cpp
    src/xHal_Rpi5CarGpioHandler.cpp
    src/xHal_Rpi5CarGpioSimulationArguments.cpp)
if(XWALK_GPIO_SIMULATION_BACKEND STREQUAL "stub")
    list(APPEND XWALK_GPIO_SIMULATION_SOURCES
        src/xHal_Rpi5CarGpioDeviceFactoryStub.cpp
        src/xHal_Rpi5CarGpioHostStub.cpp)
else()
    list(APPEND XWALK_GPIO_SIMULATION_SOURCES
        src/xHal_Rpi5CarGpioDeviceFactoryHardware.cpp)
endif()

add_executable(xWalkGpioSimulation ${XWALK_GPIO_SIMULATION_SOURCES})
target_include_directories(xWalkGpioSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkGpioSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkGpioSimulation PRIVATE
    XWALK_GPIO_SIMULATION_DEVICE_PATH="${XWALK_GPIO_SIMULATION_DEVICE}"
    XWALK_GPIO_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GPIO_TRACE_CONFIG}"
    XWALK_GPIO_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkGpioSimulation.log")
target_link_libraries(xWalkGpioSimulation PRIVATE xWalkGpioLinux xWalkTrace)
add_dependencies(xWalkGpioSimulation xWalkGpioSimulationTraceConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkGpioSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
