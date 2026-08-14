cmake_minimum_required(VERSION 3.16)
project(xWalkLineTrackerSimulation LANGUAGES CXX)
set(XWALK_ADC_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_ADC_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_LINE_TRACKER_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_LINE_TRACKER_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkLineTracker)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_LINE_TRACKER_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_LINE_TRACKER_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarLineTrackerTraceConfig.py)
set(XWALK_LINE_TRACKER_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkLineTrackerTrace.xml)
add_custom_command(OUTPUT ${XWALK_LINE_TRACKER_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_LINE_TRACKER_TRACE_GENERATOR}
        --input ${XWALK_LINE_TRACKER_TRACE_INVENTORY}
        --output ${XWALK_LINE_TRACKER_TRACE_CONFIG}
    DEPENDS ${XWALK_LINE_TRACKER_TRACE_INVENTORY} ${XWALK_LINE_TRACKER_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkLineTracker trace catalogue" VERBATIM)
add_custom_target(xWalkLineTrackerSimulationTraceConfig ALL
    DEPENDS ${XWALK_LINE_TRACKER_TRACE_CONFIG})
add_dependencies(xWalkLineTrackerSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkLineTrackerSimulation
    src/main.cpp
    src/xHal_Rpi5CarLineTrackerHostStub.cpp
    src/xHal_Rpi5CarLineTrackerSimulation.cpp
    src/xHal_Rpi5CarLineTrackerSimulationArguments.cpp)
target_include_directories(xWalkLineTrackerSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLineTrackerSimulation PRIVATE cxx_std_17)
set(XWALK_LINE_TRACKER_SIMULATION_LOG
    ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkLineTrackerSimulation.log)
target_compile_definitions(xWalkLineTrackerSimulation PRIVATE
    XWALK_LINE_TRACKER_SIMULATION_TRACE_CONFIG_PATH="${XWALK_LINE_TRACKER_TRACE_CONFIG}"
    XWALK_LINE_TRACKER_SIMULATION_TRACE_LOG_PATH="${XWALK_LINE_TRACKER_SIMULATION_LOG}")
target_link_libraries(xWalkLineTrackerSimulation PRIVATE xWalkLineTracker xWalkTrace)
add_dependencies(xWalkLineTrackerSimulation xWalkLineTrackerSimulationTraceConfig)
