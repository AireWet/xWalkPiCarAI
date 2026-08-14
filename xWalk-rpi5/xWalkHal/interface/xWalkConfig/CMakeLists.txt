cmake_minimum_required(VERSION 3.16)
project(xWalkConfig LANGUAGES CXX)

option(XWALK_CONFIG_BUILD_HOST_TESTS "Build xWalkConfig host tests" OFF)

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkConfig STATIC
    src/xHal_Rpi5CarConfig.cpp
    src/xHal_Rpi5CarConfigLifecycle.cpp
    src/xHal_Rpi5CarConfigStore.cpp
    src/xHal_Rpi5CarConfigStoreLifecycle.cpp
)

target_include_directories(xWalkConfig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkConfig PUBLIC cxx_std_17)
target_link_libraries(xWalkConfig
    PUBLIC xWalkLibraryCommon
    PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkConfig PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_CONFIG_BUILD_HOST_TESTS)
    include(CTest)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_CONFIG_TEST_TRACE_INVENTORY
        ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_CONFIG_TEST_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarConfigTraceConfig.py)
    set(XWALK_CONFIG_TEST_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkConfigTrace.xml)
    add_custom_command(
        OUTPUT ${XWALK_CONFIG_TEST_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_CONFIG_TEST_TRACE_GENERATOR}
            --input ${XWALK_CONFIG_TEST_TRACE_INVENTORY}
            --output ${XWALK_CONFIG_TEST_TRACE_CONFIG}
        DEPENDS ${XWALK_CONFIG_TEST_TRACE_INVENTORY}
            ${XWALK_CONFIG_TEST_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkConfig host-test catalogue"
        VERBATIM)
    add_custom_target(xWalkConfigTestTraceConfig
        DEPENDS ${XWALK_CONFIG_TEST_TRACE_CONFIG})
    add_dependencies(xWalkConfigTestTraceConfig xWalkTraceMetadata)

    add_executable(xWalkConfigTest
        test/src/xHal_Rpi5CarConfigTest.cpp
        test/src/xHal_Rpi5CarConfigTestSupport.cpp
        simulation/src/xHal_Rpi5CarConfigSimulationArguments.cpp)
    target_include_directories(xWalkConfigTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    target_compile_definitions(xWalkConfigTest PRIVATE
        XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH="${XWALK_CONFIG_TEST_TRACE_CONFIG}"
        XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkConfigTest.log")
    target_link_libraries(xWalkConfigTest PRIVATE xWalkConfig xWalkTrace)
    add_dependencies(xWalkConfigTest xWalkConfigTestTraceConfig)
    add_test(NAME xWalkConfigHostTest COMMAND xWalkConfigTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/section-config/config.ini)
    set_tests_properties(xWalkConfigHostTest PROPERTIES LABELS "host")

    add_executable(xWalkConfigStoreTest test/src/xHal_Rpi5CarConfigStoreTest.cpp)
    target_include_directories(xWalkConfigStoreTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    target_compile_definitions(xWalkConfigStoreTest PRIVATE
        XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH="${XWALK_CONFIG_TEST_TRACE_CONFIG}"
        XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkConfigStoreTest.log")
    target_link_libraries(xWalkConfigStoreTest PRIVATE xWalkConfig xWalkTrace)
    add_dependencies(xWalkConfigStoreTest xWalkConfigTestTraceConfig)
    add_test(NAME xWalkConfigStoreHostTest COMMAND xWalkConfigStoreTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/config-store/config-store.config)
    set_tests_properties(xWalkConfigStoreHostTest PROPERTIES LABELS "host")
endif()
