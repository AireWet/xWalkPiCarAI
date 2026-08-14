cmake_minimum_required(VERSION 3.16)
project(xWalkPwmSimulation LANGUAGES CXX)
set(XWALK_I2C_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_I2C_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware dependency tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkPwm)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_PWM_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_PWM_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarPwmTraceConfig.py)
set(XWALK_PWM_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkPwmTrace.xml)
add_custom_command(
    OUTPUT ${XWALK_PWM_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_PWM_TRACE_GENERATOR}
        --input ${XWALK_PWM_TRACE_INVENTORY} --output ${XWALK_PWM_TRACE_CONFIG}
    DEPENDS ${XWALK_PWM_TRACE_INVENTORY} ${XWALK_PWM_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkPwm trace catalogue" VERBATIM)
add_custom_target(xWalkPwmSimulationTraceConfig ALL DEPENDS ${XWALK_PWM_TRACE_CONFIG})
add_dependencies(xWalkPwmSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkPwmSimulation
    src/main.cpp
    src/xHal_Rpi5CarPwmHostStub.cpp
    src/xHal_Rpi5CarPwmSimulation.cpp
    src/xHal_Rpi5CarPwmSimulationArguments.cpp)
target_include_directories(xWalkPwmSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkPwmSimulation PRIVATE cxx_std_17)
set(XWALK_PWM_SIMULATION_LOG
    "${CMAKE_CURRENT_BINARY_DIR}/log/xWalkPwmSimulation.log")
target_compile_definitions(xWalkPwmSimulation PRIVATE
    XWALK_PWM_SIMULATION_TRACE_CONFIG_PATH="${XWALK_PWM_TRACE_CONFIG}"
    XWALK_PWM_SIMULATION_TRACE_LOG_PATH="${XWALK_PWM_SIMULATION_LOG}")
target_link_libraries(xWalkPwmSimulation PRIVATE xWalkPwm xWalkTrace)
add_dependencies(xWalkPwmSimulation xWalkPwmSimulationTraceConfig)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkPwmSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
