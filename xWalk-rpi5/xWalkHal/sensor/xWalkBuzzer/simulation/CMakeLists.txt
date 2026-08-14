cmake_minimum_required(VERSION 3.16)
project(xWalkBuzzerSimulation LANGUAGES CXX)
set(XWALK_GPIO_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_GPIO_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_PWM_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_PWM_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_BUZZER_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_BUZZER_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkBuzzer)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_BUZZER_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_BUZZER_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarBuzzerTraceConfig.py)
set(XWALK_BUZZER_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkBuzzerTrace.xml)
add_custom_command(OUTPUT ${XWALK_BUZZER_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_BUZZER_TRACE_GENERATOR}
        --input ${XWALK_BUZZER_TRACE_INVENTORY} --output ${XWALK_BUZZER_TRACE_CONFIG}
    DEPENDS ${XWALK_BUZZER_TRACE_INVENTORY} ${XWALK_BUZZER_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkBuzzer trace catalogue" VERBATIM)
add_custom_target(xWalkBuzzerSimulationTraceConfig ALL DEPENDS ${XWALK_BUZZER_TRACE_CONFIG})
add_dependencies(xWalkBuzzerSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkBuzzerSimulation
    src/main.cpp
    src/xHal_Rpi5CarBuzzerHostStub.cpp
    src/xHal_Rpi5CarBuzzerSimulation.cpp
    src/xHal_Rpi5CarBuzzerSimulationArguments.cpp)
target_include_directories(xWalkBuzzerSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkBuzzerSimulation PRIVATE cxx_std_17)
set(XWALK_BUZZER_SIMULATION_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkBuzzerSimulation.log)
target_compile_definitions(xWalkBuzzerSimulation PRIVATE
    XWALK_BUZZER_SIMULATION_TRACE_CONFIG_PATH="${XWALK_BUZZER_TRACE_CONFIG}"
    XWALK_BUZZER_SIMULATION_TRACE_LOG_PATH="${XWALK_BUZZER_SIMULATION_LOG}")
target_link_libraries(xWalkBuzzerSimulation PRIVATE xWalkBuzzer xWalkTrace)
add_dependencies(xWalkBuzzerSimulation xWalkBuzzerSimulationTraceConfig)
