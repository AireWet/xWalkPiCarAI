cmake_minimum_required(VERSION 3.16)
project(xWalkLedSimulation LANGUAGES CXX)
set(XWALK_GPIO_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_GPIO_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_PWM_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_PWM_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_LED_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_LED_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkLed)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_LED_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_LED_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarLedTraceConfig.py)
set(XWALK_LED_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkLedTrace.xml)
add_custom_command(OUTPUT ${XWALK_LED_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_LED_TRACE_GENERATOR}
        --input ${XWALK_LED_TRACE_INVENTORY} --output ${XWALK_LED_TRACE_CONFIG}
    DEPENDS ${XWALK_LED_TRACE_INVENTORY} ${XWALK_LED_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkLed trace catalogue" VERBATIM)
add_custom_target(xWalkLedSimulationTraceConfig ALL DEPENDS ${XWALK_LED_TRACE_CONFIG})
add_dependencies(xWalkLedSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkLedSimulation
    src/main.cpp
    src/xHal_Rpi5CarLedHostStub.cpp
    src/xHal_Rpi5CarLedSimulation.cpp
    src/xHal_Rpi5CarLedSimulationArguments.cpp)
target_include_directories(xWalkLedSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLedSimulation PRIVATE cxx_std_17)
set(XWALK_LED_SIMULATION_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkLedSimulation.log)
target_compile_definitions(xWalkLedSimulation PRIVATE
    XWALK_LED_SIMULATION_TRACE_CONFIG_PATH="${XWALK_LED_TRACE_CONFIG}"
    XWALK_LED_SIMULATION_TRACE_LOG_PATH="${XWALK_LED_SIMULATION_LOG}")
target_link_libraries(xWalkLedSimulation PRIVATE xWalkLed xWalkTrace)
add_dependencies(xWalkLedSimulation xWalkLedSimulationTraceConfig)
