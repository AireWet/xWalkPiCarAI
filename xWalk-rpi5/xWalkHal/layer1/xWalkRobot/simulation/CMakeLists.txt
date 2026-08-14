cmake_minimum_required(VERSION 3.16)
project(xWalkRobotSimulation LANGUAGES CXX)
set(XWALK_ROBOT_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_ROBOT_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkRobot)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_ROBOT_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_ROBOT_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarRobotTraceConfig.py)
set(XWALK_ROBOT_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkRobotTrace.xml)
add_custom_command(OUTPUT ${XWALK_ROBOT_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_ROBOT_TRACE_GENERATOR}
        --input ${XWALK_ROBOT_TRACE_INVENTORY} --output ${XWALK_ROBOT_TRACE_CONFIG}
    DEPENDS ${XWALK_ROBOT_TRACE_INVENTORY} ${XWALK_ROBOT_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkRobot trace catalogue" VERBATIM)
add_custom_target(xWalkRobotSimulationTraceConfig ALL DEPENDS ${XWALK_ROBOT_TRACE_CONFIG})
add_dependencies(xWalkRobotSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkRobotSimulation
    src/main.cpp
    src/xHal_Rpi5CarRobotHostStub.cpp
    src/xHal_Rpi5CarRobotSimulation.cpp
    src/xHal_Rpi5CarRobotSimulationArguments.cpp)
target_include_directories(xWalkRobotSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkRobotSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkRobotSimulation PRIVATE
    XWALK_ROBOT_SIMULATION_TRACE_CONFIG_PATH="${XWALK_ROBOT_TRACE_CONFIG}"
    XWALK_ROBOT_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkRobotSimulation.log"
    XWALK_ROBOT_SIMULATION_STORE_PATH="${CMAKE_CURRENT_BINARY_DIR}/data/robot.config")
target_link_libraries(xWalkRobotSimulation PRIVATE xWalkRobot xWalkTrace)
add_dependencies(xWalkRobotSimulation xWalkRobotSimulationTraceConfig)
