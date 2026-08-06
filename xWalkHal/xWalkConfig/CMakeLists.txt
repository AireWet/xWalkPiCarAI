cmake_minimum_required(VERSION 3.16)
project(xWalkConfig LANGUAGES CXX)

option(XWALK_CONFIG_BUILD_HOST_TESTS "Build xWalkConfig host tests" OFF)

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

add_library(xWalkConfig STATIC
    src/xHal_Rpi5CarConfig.cpp
    src/xHal_Rpi5CarConfigLifecycle.cpp
    src/xHal_Rpi5CarConfigStore.cpp
    src/xHal_Rpi5CarConfigStoreLifecycle.cpp
)

target_include_directories(xWalkConfig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkConfig PUBLIC cxx_std_17)
target_link_libraries(xWalkConfig PUBLIC xWalkLibraryCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkConfig PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_CONFIG_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkConfigTest test/src/xHal_Rpi5CarConfigTest.cpp)
    target_link_libraries(xWalkConfigTest PRIVATE xWalkConfig)
    add_test(NAME xWalkConfigHostTest COMMAND xWalkConfigTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/section-config/config.ini)
    set_tests_properties(xWalkConfigHostTest PROPERTIES LABELS "host")

    add_executable(xWalkConfigStoreTest test/src/xHal_Rpi5CarConfigStoreTest.cpp)
    target_link_libraries(xWalkConfigStoreTest PRIVATE xWalkConfig)
    add_test(NAME xWalkConfigStoreHostTest COMMAND xWalkConfigStoreTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/config-store/config-store.config)
    set_tests_properties(xWalkConfigStoreHostTest PROPERTIES LABELS "host")
endif()
