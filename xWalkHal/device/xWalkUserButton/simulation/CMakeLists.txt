cmake_minimum_required(VERSION 3.16)
project(xWalkUserButtonSimulation LANGUAGES CXX)
set(XWALK_GPIO_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_GPIO_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_USER_BUTTON_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkUserButton)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_USER_BUTTON_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_USER_BUTTON_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarUserButtonTraceConfig.py)
set(XWALK_USER_BUTTON_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkUserButtonTrace.xml)
add_custom_command(OUTPUT ${XWALK_USER_BUTTON_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_USER_BUTTON_TRACE_GENERATOR}
        --input ${XWALK_USER_BUTTON_TRACE_INVENTORY}
        --output ${XWALK_USER_BUTTON_TRACE_CONFIG}
    DEPENDS ${XWALK_USER_BUTTON_TRACE_INVENTORY} ${XWALK_USER_BUTTON_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkUserButton trace catalogue" VERBATIM)
add_custom_target(xWalkUserButtonSimulationTraceConfig ALL
    DEPENDS ${XWALK_USER_BUTTON_TRACE_CONFIG})
add_dependencies(xWalkUserButtonSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkUserButtonSimulation
    src/main.cpp
    src/xHal_Rpi5CarUserButtonHostStub.cpp
    src/xHal_Rpi5CarUserButtonSimulation.cpp
    src/xHal_Rpi5CarUserButtonSimulationArguments.cpp)
target_include_directories(xWalkUserButtonSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkUserButtonSimulation PRIVATE cxx_std_17)
set(XWALK_USER_BUTTON_SIMULATION_LOG
    ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkUserButtonSimulation.log)
target_compile_definitions(xWalkUserButtonSimulation PRIVATE
    XWALK_USER_BUTTON_SIMULATION_TRACE_CONFIG_PATH="${XWALK_USER_BUTTON_TRACE_CONFIG}"
    XWALK_USER_BUTTON_SIMULATION_TRACE_LOG_PATH="${XWALK_USER_BUTTON_SIMULATION_LOG}")
target_link_libraries(xWalkUserButtonSimulation PRIVATE xWalkUserButton xWalkTrace)
add_dependencies(xWalkUserButtonSimulation xWalkUserButtonSimulationTraceConfig)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkUserButtonSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
