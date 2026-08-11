cmake_minimum_required(VERSION 3.16)
project(xWalkMotorSimulation LANGUAGES CXX)
set(XWALK_PWM_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_PWM_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_GPIO_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_GPIO_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_MOTOR_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_MOTOR_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkMotor)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_MOTOR_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_MOTOR_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarMotorTraceConfig.py)
set(XWALK_MOTOR_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkMotorTrace.xml)
add_custom_command(OUTPUT ${XWALK_MOTOR_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_MOTOR_TRACE_GENERATOR}
        --input ${XWALK_MOTOR_TRACE_INVENTORY} --output ${XWALK_MOTOR_TRACE_CONFIG}
    DEPENDS ${XWALK_MOTOR_TRACE_INVENTORY} ${XWALK_MOTOR_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkMotor trace catalogue" VERBATIM)
add_custom_target(xWalkMotorSimulationTraceConfig ALL DEPENDS ${XWALK_MOTOR_TRACE_CONFIG})
add_dependencies(xWalkMotorSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkMotorSimulation
    src/main.cpp
    src/xHal_Rpi5CarMotorHostStub.cpp
    src/xHal_Rpi5CarMotorSimulation.cpp
    src/xHal_Rpi5CarMotorSimulationArguments.cpp)
target_include_directories(xWalkMotorSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkMotorSimulation PRIVATE cxx_std_17)
set(XWALK_MOTOR_SIMULATION_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkMotorSimulation.log)
target_compile_definitions(xWalkMotorSimulation PRIVATE
    XWALK_MOTOR_SIMULATION_TRACE_CONFIG_PATH="${XWALK_MOTOR_TRACE_CONFIG}"
    XWALK_MOTOR_SIMULATION_TRACE_LOG_PATH="${XWALK_MOTOR_SIMULATION_LOG}")
target_link_libraries(xWalkMotorSimulation PRIVATE xWalkMotor xWalkTrace)
add_dependencies(xWalkMotorSimulation xWalkMotorSimulationTraceConfig)
