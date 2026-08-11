find_package(GTest CONFIG REQUIRED)
find_library(XWALK_INTERFACE_GROUP_GMOCK_LIBRARY NAMES gmock REQUIRED)
find_path(XWALK_INTERFACE_GROUP_GMOCK_INCLUDE_DIRECTORY gmock/gmock.h
    HINTS /usr/src/googletest/googlemock/include REQUIRED)

add_executable(xWalkInterfaceGroupTest
    src/xHal_Rpi5CarInterfaceGroupTest.cpp
    src/xHal_Rpi5CarInterfaceGroupTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAudio/hardware/test/src/xHal_Rpi5CarAudioAlsaTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLanguageModel/test/src/xHal_Rpi5CarLanguageModelTestSupport.cpp)
target_link_libraries(xWalkInterfaceGroupTest PRIVATE xWalkTrace)
target_include_directories(xWalkInterfaceGroupTest SYSTEM PRIVATE
    ${XWALK_INTERFACE_GROUP_GMOCK_INCLUDE_DIRECTORY})
target_include_directories(xWalkInterfaceGroupTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAudio/hardware/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLanguageModel/test/include)
target_compile_features(xWalkInterfaceGroupTest PRIVATE cxx_std_17)
target_compile_definitions(xWalkInterfaceGroupTest PRIVATE
    XWALK_INTERFACE_GROUP_TEST_DATA_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/test-data")
target_link_libraries(xWalkInterfaceGroupTest PRIVATE
    GTest::gtest_main
    ${XWALK_INTERFACE_GROUP_GMOCK_LIBRARY}
    xWalkAudioAlsa
    xWalkConfig
    xWalkGpio
    xWalkI2c
    xWalkLanguageModel
    xWalkSpi
    xWalkUtils)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkInterfaceGroupTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_test(NAME xWalkInterfaceGroupTest COMMAND xWalkInterfaceGroupTest)
set_tests_properties(xWalkInterfaceGroupTest PROPERTIES
    LABELS "host;interface-group;group-tests")
