cmake_minimum_required(VERSION 3.16)
project(xWalkAdxl345Simulation LANGUAGES CXX)
set(XWALK_I2C_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_I2C_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware dependency tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkAdxl345)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_ADXL345_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_ADXL345_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarAdxl345TraceConfig.py)
set(XWALK_ADXL345_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkAdxl345Trace.xml)
add_custom_command(OUTPUT ${XWALK_ADXL345_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_ADXL345_TRACE_GENERATOR}
        --input ${XWALK_ADXL345_TRACE_INVENTORY} --output ${XWALK_ADXL345_TRACE_CONFIG}
    DEPENDS ${XWALK_ADXL345_TRACE_INVENTORY} ${XWALK_ADXL345_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkAdxl345 trace catalogue" VERBATIM)
add_custom_target(xWalkAdxl345SimulationTraceConfig ALL
    DEPENDS ${XWALK_ADXL345_TRACE_CONFIG})
add_dependencies(xWalkAdxl345SimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkAdxl345Simulation
    src/main.cpp
    src/xHal_Rpi5CarAdxl345HostStub.cpp
    src/xHal_Rpi5CarAdxl345Simulation.cpp
    src/xHal_Rpi5CarAdxl345SimulationArguments.cpp)
target_include_directories(xWalkAdxl345Simulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAdxl345Simulation PRIVATE cxx_std_17)
set(XWALK_ADXL345_SIMULATION_LOG
    ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkAdxl345Simulation.log)
target_compile_definitions(xWalkAdxl345Simulation PRIVATE
    XWALK_ADXL345_SIMULATION_TRACE_CONFIG_PATH="${XWALK_ADXL345_TRACE_CONFIG}"
    XWALK_ADXL345_SIMULATION_TRACE_LOG_PATH="${XWALK_ADXL345_SIMULATION_LOG}")
target_link_libraries(xWalkAdxl345Simulation PRIVATE xWalkAdxl345 xWalkTrace)
add_dependencies(xWalkAdxl345Simulation xWalkAdxl345SimulationTraceConfig)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkAdxl345Simulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
