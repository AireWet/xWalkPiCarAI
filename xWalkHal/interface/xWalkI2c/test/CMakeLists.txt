find_package(GTest CONFIG REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(XWALK_I2C_TEST_TRACE_INVENTORY
    ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_I2C_TEST_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/config/xHal_Rpi5CarI2cTraceConfig.py)
set(XWALK_I2C_TEST_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkI2cTrace.xml)

add_custom_command(
    OUTPUT ${XWALK_I2C_TEST_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_I2C_TEST_TRACE_GENERATOR}
        --input ${XWALK_I2C_TEST_TRACE_INVENTORY}
        --output ${XWALK_I2C_TEST_TRACE_CONFIG}
    DEPENDS ${XWALK_I2C_TEST_TRACE_INVENTORY}
        ${XWALK_I2C_TEST_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkI2c host-test catalogue"
    VERBATIM)
add_custom_target(xWalkI2cTestTraceConfig
    DEPENDS ${XWALK_I2C_TEST_TRACE_CONFIG})
add_dependencies(xWalkI2cTestTraceConfig xWalkTraceMetadata)

add_executable(xWalkI2cTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/src/xHal_Rpi5CarI2cHostStub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/src/xHal_Rpi5CarI2cSimulationArguments.cpp
    src/xHal_Rpi5CarI2cTest.cpp)
set_target_properties(xWalkI2cTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_include_directories(xWalkI2cTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/include)
target_compile_features(xWalkI2cTest PRIVATE cxx_std_17)
target_compile_definitions(xWalkI2cTest PRIVATE
    XWALK_I2C_SIMULATION_TRACE_CONFIG_PATH="${XWALK_I2C_TEST_TRACE_CONFIG}"
    XWALK_I2C_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkI2cTest.log")
target_link_libraries(xWalkI2cTest PRIVATE
    GTest::gtest_main xWalkI2cLinux xWalkTrace)
add_dependencies(xWalkI2cTest xWalkI2cTestTraceConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkI2cTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_test(NAME xWalkI2cHostTest COMMAND xWalkI2cTest)
set_tests_properties(xWalkI2cHostTest PROPERTIES LABELS "host")
