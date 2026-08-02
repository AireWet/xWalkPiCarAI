cmake_minimum_required(VERSION 3.16)
project(xWalkSpiTransfer LANGUAGES CXX)

option(XWALK_SPI_TRANSFER_BUILD_HOST_TESTS
    "Build xWalkSpiTransfer host tests" OFF)

if(NOT TARGET xWalkSpi)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkHal/xWalkSpi
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkSpi)
endif()

add_library(xWalkSpiTransfer STATIC
    src/xAgent_Rpi5CarSpiTransfer.cpp)
add_library(xWalk::SpiTransfer ALIAS xWalkSpiTransfer)
target_include_directories(xWalkSpiTransfer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkSpiTransfer PUBLIC cxx_std_17)
target_link_libraries(xWalkSpiTransfer PUBLIC xWalkSpi)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkSpiTransfer PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_SPI_TRANSFER_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkSpiTransferTest
        test/src/xAgent_Rpi5CarSpiTransferTest.cpp)
    target_link_libraries(xWalkSpiTransferTest PRIVATE xWalkSpiTransfer)
    add_test(NAME xWalkSpiTransferHostTest COMMAND xWalkSpiTransferTest)
    set_tests_properties(xWalkSpiTransferHostTest PROPERTIES LABELS "host")
endif()
