cmake_minimum_required(VERSION 3.16)
project(xWalkBoardControlSimulation LANGUAGES CXX)
set(XWALK_BOARD_CONTROL_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkBoardControl)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_BOARD_CONTROL_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_BOARD_CONTROL_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarBoardControlTraceConfig.py)
set(XWALK_BOARD_CONTROL_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkBoardControlTrace.xml)
add_custom_command(OUTPUT ${XWALK_BOARD_CONTROL_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_BOARD_CONTROL_TRACE_GENERATOR}
        --input ${XWALK_BOARD_CONTROL_TRACE_INVENTORY} --output ${XWALK_BOARD_CONTROL_TRACE_CONFIG}
    DEPENDS ${XWALK_BOARD_CONTROL_TRACE_INVENTORY} ${XWALK_BOARD_CONTROL_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkBoardControl trace catalogue" VERBATIM)
add_custom_target(xWalkBoardControlSimulationTraceConfig ALL
    DEPENDS ${XWALK_BOARD_CONTROL_TRACE_CONFIG})
add_dependencies(xWalkBoardControlSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkBoardControlSimulation
    src/main.cpp
    src/xHal_Rpi5CarBoardControlHostStub.cpp
    src/xHal_Rpi5CarBoardControlSimulation.cpp
    src/xHal_Rpi5CarBoardControlSimulationArguments.cpp)
target_include_directories(xWalkBoardControlSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkBoardControlSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkBoardControlSimulation PRIVATE
    XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH="${XWALK_BOARD_CONTROL_TRACE_CONFIG}"
    XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkBoardControlSimulation.log"
    XWALK_BOARD_CONTROL_SIMULATION_DEVICE_TREE_PATH="${CMAKE_CURRENT_BINARY_DIR}/device-tree")
target_link_libraries(xWalkBoardControlSimulation PRIVATE xWalkBoardControl xWalkTrace)
add_dependencies(xWalkBoardControlSimulation xWalkBoardControlSimulationTraceConfig)
