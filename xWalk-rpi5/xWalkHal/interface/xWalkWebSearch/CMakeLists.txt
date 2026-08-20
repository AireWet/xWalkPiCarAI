cmake_minimum_required(VERSION 3.16)
project(xWalkWebSearch LANGUAGES CXX)

option(XWALK_WEB_SEARCH_BUILD_HOST_TESTS "Build xWalkWebSearch host tests" ${XWALK_HAL_BUILD_HOST})

find_package(CURL REQUIRED)
if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkWebSearch STATIC
    src/xHal_Rpi5CarWebSearch.cpp
    src/xHal_Rpi5CarWebSearchSystem.cpp)
add_library(xWalk::WebSearch ALIAS xWalkWebSearch)
target_include_directories(xWalkWebSearch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkWebSearch PUBLIC cxx_std_17)
target_link_libraries(xWalkWebSearch PUBLIC xWalkLibraryCommon PRIVATE xWalkTrace CURL::libcurl)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkWebSearch PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_WEB_SEARCH_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkWebSearchTest
        test/src/xHal_Rpi5CarWebSearchTest.cpp
        test/src/xHal_Rpi5CarWebSearchTestSupport.cpp)
    target_include_directories(xWalkWebSearchTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_link_libraries(xWalkWebSearchTest PRIVATE xWalkWebSearch xWalkTrace)
    add_test(NAME xWalkWebSearchHostTest COMMAND xWalkWebSearchTest)
    set_tests_properties(xWalkWebSearchHostTest PROPERTIES LABELS "host")
endif()
