cmake_minimum_required(VERSION 3.16)
project(xWalkCameraSimulation LANGUAGES CXX)
set(XWALK_CAMERA_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_CAMERA_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_CAMERA_BUILD_LINUX_BACKEND OFF CACHE BOOL "Disable Linux camera backend" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkCamera)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_CAMERA_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_CAMERA_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarCameraTraceConfig.py)
set(XWALK_CAMERA_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkCameraTrace.xml)
add_custom_command(OUTPUT ${XWALK_CAMERA_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_CAMERA_TRACE_GENERATOR}
        --input ${XWALK_CAMERA_TRACE_INVENTORY}
        --output ${XWALK_CAMERA_TRACE_CONFIG}
    DEPENDS ${XWALK_CAMERA_TRACE_INVENTORY} ${XWALK_CAMERA_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkCamera trace catalogue" VERBATIM)
add_custom_target(xWalkCameraSimulationTraceConfig ALL DEPENDS ${XWALK_CAMERA_TRACE_CONFIG})
add_dependencies(xWalkCameraSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkCameraSimulation
    src/main.cpp
    src/xHal_Rpi5CarCameraHostStub.cpp
    src/xHal_Rpi5CarCameraSimulation.cpp
    src/xHal_Rpi5CarCameraSimulationArguments.cpp)
target_include_directories(xWalkCameraSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkCameraSimulation PRIVATE cxx_std_17)
set(XWALK_CAMERA_SIMULATION_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkCameraSimulation.log)
target_compile_definitions(xWalkCameraSimulation PRIVATE
    XWALK_CAMERA_SIMULATION_TRACE_CONFIG_PATH="${XWALK_CAMERA_TRACE_CONFIG}"
    XWALK_CAMERA_SIMULATION_TRACE_LOG_PATH="${XWALK_CAMERA_SIMULATION_LOG}")
target_link_libraries(xWalkCameraSimulation PRIVATE xWalkCamera xWalkTrace)
add_dependencies(xWalkCameraSimulation xWalkCameraSimulationTraceConfig)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkCameraSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
