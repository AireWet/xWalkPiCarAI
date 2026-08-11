cmake_minimum_required(VERSION 3.16)
project(xWalkServoSimulation LANGUAGES CXX)
set(XWALK_PWM_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_PWM_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware dependency tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkServo)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_SERVO_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_SERVO_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarServoTraceConfig.py)
set(XWALK_SERVO_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkServoTrace.xml)
add_custom_command(OUTPUT ${XWALK_SERVO_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_SERVO_TRACE_GENERATOR}
        --input ${XWALK_SERVO_TRACE_INVENTORY} --output ${XWALK_SERVO_TRACE_CONFIG}
    DEPENDS ${XWALK_SERVO_TRACE_INVENTORY} ${XWALK_SERVO_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkServo trace catalogue" VERBATIM)
add_custom_target(xWalkServoSimulationTraceConfig ALL DEPENDS ${XWALK_SERVO_TRACE_CONFIG})
add_dependencies(xWalkServoSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkServoSimulation
    src/main.cpp
    src/xHal_Rpi5CarServoHostStub.cpp
    src/xHal_Rpi5CarServoSimulation.cpp
    src/xHal_Rpi5CarServoSimulationArguments.cpp)
target_include_directories(xWalkServoSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkServoSimulation PRIVATE cxx_std_17)
set(XWALK_SERVO_SIMULATION_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkServoSimulation.log)
target_compile_definitions(xWalkServoSimulation PRIVATE
    XWALK_SERVO_SIMULATION_TRACE_CONFIG_PATH="${XWALK_SERVO_TRACE_CONFIG}"
    XWALK_SERVO_SIMULATION_TRACE_LOG_PATH="${XWALK_SERVO_SIMULATION_LOG}")
target_link_libraries(xWalkServoSimulation PRIVATE xWalkServo xWalkTrace)
add_dependencies(xWalkServoSimulation xWalkServoSimulationTraceConfig)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkServoSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
