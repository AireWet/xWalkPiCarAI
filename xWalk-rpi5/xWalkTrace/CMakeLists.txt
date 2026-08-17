cmake_minimum_required(VERSION 3.16)
project(xWalkTrace LANGUAGES CXX)

option(XWALK_TRACE_BUILD_HOST_TESTS "Build xWalkTrace host tests" OFF)
option(XWALK_TRACE_BUILD_HARDWARE_TESTS "Build xWalkTrace target compile tests" OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLibrary/XWalkDependencies.cmake)

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSON_C REQUIRED IMPORTED_TARGET json-c)

set(XWALK_TRACE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(XWALK_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/pre-compiler/xHal_Rpi5CarTracePreCompiler.py)
set(XWALK_TRACE_PRIORITY_CONFIGS
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xWalkTraceCriticalPriorities.json
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xWalkTraceErrorPriorities.json
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xWalkTraceWarningPriorities.json
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xWalkTraceInfoPriorities.json)
set(XWALK_TRACE_GENERATED_DIRECTORY ${CMAKE_BINARY_DIR}/generated)
set(XWALK_TRACE_GENERATED_XML ${XWALK_TRACE_GENERATED_DIRECTORY}/xwalk-traces.xml)
set(XWALK_TRACE_GENERATED_INCLUDE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/generated/include)
set(XWALK_TRACE_LOG_DIRECTORY ${CMAKE_BINARY_DIR}/log)
file(MAKE_DIRECTORY ${XWALK_TRACE_GENERATED_INCLUDE_DIRECTORY})
file(MAKE_DIRECTORY ${XWALK_TRACE_LOG_DIRECTORY})
configure_file(config/xHal_Rpi5CarTraceBuildConfig.h.in
    ${XWALK_TRACE_GENERATED_INCLUDE_DIRECTORY}/xHal_Rpi5CarTraceBuildConfig.h @ONLY)
set(XWALK_TRACE_SOURCE_ROOTS
    ${XWALK_TRACE_PROJECT_ROOT})

set(XWALK_TRACE_SCANNED_SOURCES)
foreach(XWALK_TRACE_SOURCE_ROOT IN LISTS XWALK_TRACE_SOURCE_ROOTS)
    file(GLOB_RECURSE XWALK_TRACE_ROOT_SOURCES CONFIGURE_DEPENDS
        ${XWALK_TRACE_SOURCE_ROOT}/*.c
        ${XWALK_TRACE_SOURCE_ROOT}/*.cc
        ${XWALK_TRACE_SOURCE_ROOT}/*.cpp
        ${XWALK_TRACE_SOURCE_ROOT}/*.cxx
        ${XWALK_TRACE_SOURCE_ROOT}/*.h
        ${XWALK_TRACE_SOURCE_ROOT}/*.hpp)
    list(APPEND XWALK_TRACE_SCANNED_SOURCES ${XWALK_TRACE_ROOT_SOURCES})
endforeach()
list(FILTER XWALK_TRACE_SCANNED_SOURCES EXCLUDE REGEX
    "/(build|build-[^/]*|cmake-build-[^/]*)/")

add_custom_command(
    OUTPUT ${XWALK_TRACE_GENERATED_XML}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_TRACE_GENERATOR}
        --project-root ${XWALK_TRACE_PROJECT_ROOT}
        --source-root ${XWALK_TRACE_PROJECT_ROOT}
        --priority-config ${CMAKE_CURRENT_SOURCE_DIR}/config/xWalkTraceCriticalPriorities.json
        --priority-config ${CMAKE_CURRENT_SOURCE_DIR}/config/xWalkTraceErrorPriorities.json
        --priority-config ${CMAKE_CURRENT_SOURCE_DIR}/config/xWalkTraceWarningPriorities.json
        --priority-config ${CMAKE_CURRENT_SOURCE_DIR}/config/xWalkTraceInfoPriorities.json
        --output ${XWALK_TRACE_GENERATED_XML}
    DEPENDS ${XWALK_TRACE_GENERATOR} ${XWALK_TRACE_PRIORITY_CONFIGS}
        ${XWALK_TRACE_SCANNED_SOURCES}
    COMMENT "Validating xWalk trace identifiers and generating XML metadata"
    VERBATIM)
add_custom_target(xWalkTraceMetadata ALL DEPENDS ${XWALK_TRACE_GENERATED_XML})

add_library(xWalkTrace STATIC
    src/xHal_Rpi5CarTrace.cpp
    src/xHal_Rpi5CarTraceConfiguration.cpp
    src/xHal_Rpi5CarTraceControl.cpp
    src/xHal_Rpi5CarTraceFormatting.cpp
    src/xHal_Rpi5CarTraceLifecycle.cpp
)
add_dependencies(xWalkTrace xWalkTraceMetadata)

target_include_directories(xWalkTrace PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${XWALK_TRACE_GENERATED_INCLUDE_DIRECTORY})
target_compile_features(xWalkTrace PUBLIC cxx_std_17)
target_compile_definitions(xWalkTrace PRIVATE
    XWALK_TRACE_CONFIG_PATH="${XWALK_TRACE_GENERATED_XML}")
target_link_libraries(xWalkTrace PUBLIC xWalkLibraryCommon tinyxml2::tinyxml2
    PRIVATE PkgConfig::JSON_C)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkTrace PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_TRACE_BUILD_HOST_TESTS OR XWALK_TRACE_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_TRACE_BUILD_HOST_TESTS)
    add_executable(xWalkTraceTest test/src/xHal_Rpi5CarTraceTest.cpp)
    target_include_directories(xWalkTraceTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_link_libraries(xWalkTraceTest PRIVATE xWalkTrace)
    add_test(NAME xWalkTraceHostTest COMMAND xWalkTraceTest)
    set_tests_properties(xWalkTraceHostTest PROPERTIES LABELS "host")

    add_test(NAME xWalkTraceScannerHostTest
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test/xHal_Rpi5CarTracePreCompilerTest.py)
    set_tests_properties(xWalkTraceScannerHostTest PROPERTIES LABELS "host;tooling")
endif()

if(XWALK_TRACE_BUILD_HARDWARE_TESTS)
    add_executable(xWalkTraceHardwareTest
        test/hardware/src/xHal_Rpi5CarTraceHardwareTest.cpp)
    target_link_libraries(xWalkTraceHardwareTest PRIVATE xWalkTrace)
    add_test(NAME xWalkTraceTargetCompileTest COMMAND xWalkTraceHardwareTest)
    set_tests_properties(xWalkTraceTargetCompileTest PROPERTIES LABELS "hardware")
endif()
