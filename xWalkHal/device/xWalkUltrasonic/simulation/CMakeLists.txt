cmake_minimum_required(VERSION 3.16)
project(xWalkUltrasonicSimulation LANGUAGES CXX)
set(XWALK_GPIO_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_GPIO_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware dependency tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkUltrasonic)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_ULTRASONIC_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_ULTRASONIC_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarUltrasonicTraceConfig.py)
set(XWALK_ULTRASONIC_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkUltrasonicTrace.xml)
add_custom_command(OUTPUT ${XWALK_ULTRASONIC_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_ULTRASONIC_TRACE_GENERATOR}
        --input ${XWALK_ULTRASONIC_TRACE_INVENTORY}
        --output ${XWALK_ULTRASONIC_TRACE_CONFIG}
    DEPENDS ${XWALK_ULTRASONIC_TRACE_INVENTORY} ${XWALK_ULTRASONIC_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkUltrasonic trace catalogue" VERBATIM)
add_custom_target(xWalkUltrasonicSimulationTraceConfig ALL
    DEPENDS ${XWALK_ULTRASONIC_TRACE_CONFIG})
add_dependencies(xWalkUltrasonicSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkUltrasonicSimulation
    src/main.cpp
    src/xHal_Rpi5CarUltrasonicHostStub.cpp
    src/xHal_Rpi5CarUltrasonicSimulation.cpp
    src/xHal_Rpi5CarUltrasonicSimulationArguments.cpp)
target_include_directories(xWalkUltrasonicSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkUltrasonicSimulation PRIVATE cxx_std_17)
set(XWALK_ULTRASONIC_SIMULATION_LOG
    ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkUltrasonicSimulation.log)
target_compile_definitions(xWalkUltrasonicSimulation PRIVATE
    XWALK_ULTRASONIC_SIMULATION_TRACE_CONFIG_PATH="${XWALK_ULTRASONIC_TRACE_CONFIG}"
    XWALK_ULTRASONIC_SIMULATION_TRACE_LOG_PATH="${XWALK_ULTRASONIC_SIMULATION_LOG}")
target_link_libraries(xWalkUltrasonicSimulation PRIVATE xWalkUltrasonic xWalkTrace)
add_dependencies(xWalkUltrasonicSimulation xWalkUltrasonicSimulationTraceConfig)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkUltrasonicSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
