find_package(GTest CONFIG REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(XWALK_GPIO_TEST_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GPIO_TEST_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/config/xHal_Rpi5CarGpioTraceConfig.py)
set(XWALK_GPIO_TEST_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkGpioTrace.xml)

add_custom_command(
    OUTPUT ${XWALK_GPIO_TEST_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GPIO_TEST_TRACE_GENERATOR}
        --input ${XWALK_GPIO_TEST_TRACE_INVENTORY}
        --output ${XWALK_GPIO_TEST_TRACE_CONFIG}
    DEPENDS ${XWALK_GPIO_TEST_TRACE_INVENTORY} ${XWALK_GPIO_TEST_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkGpio host-test catalogue"
    VERBATIM)
add_custom_target(xWalkGpioTestTraceConfig DEPENDS ${XWALK_GPIO_TEST_TRACE_CONFIG})
add_dependencies(xWalkGpioTestTraceConfig xWalkTraceMetadata)

add_executable(xWalkGpioTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/src/xHal_Rpi5CarGpioHostStub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/src/xHal_Rpi5CarGpioSimulationArguments.cpp
    src/xHal_Rpi5CarGpioTest.cpp
    src/xHal_Rpi5CarGpioTestSupport.cpp)
set_target_properties(xWalkGpioTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_include_directories(xWalkGpioTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/include)
target_compile_features(xWalkGpioTest PRIVATE cxx_std_17)
target_compile_definitions(xWalkGpioTest PRIVATE
    XWALK_GPIO_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GPIO_TEST_TRACE_CONFIG}"
    XWALK_GPIO_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkGpioTest.log")
target_link_libraries(xWalkGpioTest PRIVATE GTest::gtest_main xWalkGpioLinux xWalkTrace)
add_dependencies(xWalkGpioTest xWalkGpioTestTraceConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkGpioTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_test(NAME xWalkGpioHostTest COMMAND xWalkGpioTest)
set_tests_properties(xWalkGpioHostTest PROPERTIES LABELS "host")
