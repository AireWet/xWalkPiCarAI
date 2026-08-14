cmake_minimum_required(VERSION 3.16)
project(xWalkSpiSimulation LANGUAGES CXX)

set(XWALK_SPI_SIMULATION_BACKEND "stub" CACHE STRING
    "Simulation device backend: stub or hardware")
set_property(CACHE XWALK_SPI_SIMULATION_BACKEND PROPERTY STRINGS stub hardware)
set(XWALK_SPI_SIMULATION_DEVICE "/dev/spidev0.0" CACHE FILEPATH
    "Linux SPI device used by the hardware backend")

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkSpi simulation requires Linux spidev")
endif()
if(NOT XWALK_SPI_SIMULATION_BACKEND STREQUAL "stub"
    AND NOT XWALK_SPI_SIMULATION_BACKEND STREQUAL "hardware")
    message(FATAL_ERROR "XWALK_SPI_SIMULATION_BACKEND must be stub or hardware")
endif()

set(XWALK_SPI_BUILD_LINUX_BACKEND ON CACHE BOOL
    "Required by the standalone simulation" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkSpi)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_SPI_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_SPI_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarSpiTraceConfig.py)
set(XWALK_SPI_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkSpiTrace.xml)

add_custom_command(
    OUTPUT ${XWALK_SPI_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_SPI_TRACE_GENERATOR}
        --input ${XWALK_SPI_TRACE_INVENTORY}
        --output ${XWALK_SPI_TRACE_CONFIG}
    DEPENDS ${XWALK_SPI_TRACE_INVENTORY} ${XWALK_SPI_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkSpi trace catalogue"
    VERBATIM)
add_custom_target(xWalkSpiSimulationTraceConfig ALL DEPENDS ${XWALK_SPI_TRACE_CONFIG})
add_dependencies(xWalkSpiSimulationTraceConfig xWalkTraceMetadata)

set(XWALK_SPI_SIMULATION_SOURCES
    src/main.cpp
    src/xHal_Rpi5CarSpiHandler.cpp
    src/xHal_Rpi5CarSpiSimulationArguments.cpp)
if(XWALK_SPI_SIMULATION_BACKEND STREQUAL "stub")
    list(APPEND XWALK_SPI_SIMULATION_SOURCES
        src/xHal_Rpi5CarSpiDeviceFactoryStub.cpp
        src/xHal_Rpi5CarSpiHostStub.cpp)
else()
    list(APPEND XWALK_SPI_SIMULATION_SOURCES
        src/xHal_Rpi5CarSpiDeviceFactoryHardware.cpp)
endif()

add_executable(xWalkSpiSimulation ${XWALK_SPI_SIMULATION_SOURCES})
target_include_directories(xWalkSpiSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkSpiSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkSpiSimulation PRIVATE
    XWALK_SPI_SIMULATION_DEVICE_PATH="${XWALK_SPI_SIMULATION_DEVICE}"
    XWALK_SPI_SIMULATION_TRACE_CONFIG_PATH="${XWALK_SPI_TRACE_CONFIG}"
    XWALK_SPI_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkSpiSimulation.log")
target_link_libraries(xWalkSpiSimulation PRIVATE xWalkSpiLinux xWalkTrace)
add_dependencies(xWalkSpiSimulation xWalkSpiSimulationTraceConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkSpiSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
