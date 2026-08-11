find_package(GTest CONFIG REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(XWALK_SPI_TEST_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_SPI_TEST_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/config/xHal_Rpi5CarSpiTraceConfig.py)
set(XWALK_SPI_TEST_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkSpiTrace.xml)

add_custom_command(
    OUTPUT ${XWALK_SPI_TEST_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_SPI_TEST_TRACE_GENERATOR}
        --input ${XWALK_SPI_TEST_TRACE_INVENTORY}
        --output ${XWALK_SPI_TEST_TRACE_CONFIG}
    DEPENDS ${XWALK_SPI_TEST_TRACE_INVENTORY} ${XWALK_SPI_TEST_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkSpi host-test catalogue"
    VERBATIM)
add_custom_target(xWalkSpiTestTraceConfig DEPENDS ${XWALK_SPI_TEST_TRACE_CONFIG})
add_dependencies(xWalkSpiTestTraceConfig xWalkTraceMetadata)

add_executable(xWalkSpiTest
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/src/xHal_Rpi5CarSpiHostStub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/src/xHal_Rpi5CarSpiSimulationArguments.cpp
    src/xHal_Rpi5CarSpiTest.cpp)
set_target_properties(xWalkSpiTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_include_directories(xWalkSpiTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../simulation/include)
target_compile_features(xWalkSpiTest PRIVATE cxx_std_17)
target_compile_definitions(xWalkSpiTest PRIVATE
    XWALK_SPI_SIMULATION_TRACE_CONFIG_PATH="${XWALK_SPI_TEST_TRACE_CONFIG}"
    XWALK_SPI_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkSpiTest.log")
target_link_libraries(xWalkSpiTest PRIVATE GTest::gtest_main xWalkSpiLinux xWalkTrace)
add_dependencies(xWalkSpiTest xWalkSpiTestTraceConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkSpiTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_test(NAME xWalkSpiHostTest COMMAND xWalkSpiTest)
set_tests_properties(xWalkSpiHostTest PROPERTIES LABELS "host")
