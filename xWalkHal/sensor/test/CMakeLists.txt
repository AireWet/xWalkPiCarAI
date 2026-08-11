find_package(GTest CONFIG REQUIRED)
find_library(XWALK_SENSOR_GROUP_GMOCK_LIBRARY NAMES gmock REQUIRED)
find_path(XWALK_SENSOR_GROUP_GMOCK_INCLUDE_DIRECTORY gmock/gmock.h
    HINTS /usr/src/googletest/googlemock/include REQUIRED)

add_executable(xWalkSensorGroupTest
    src/xHal_Rpi5CarSensorGroupTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLineTracker/test/src/xHal_Rpi5CarLineTrackerTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkMotor/test/src/xHal_Rpi5CarMotorTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLed/test/src/xHal_Rpi5CarLedTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkBuzzer/test/src/xHal_Rpi5CarBuzzerTestSupport.cpp)
target_link_libraries(xWalkSensorGroupTest PRIVATE xWalkTrace)
target_include_directories(xWalkSensorGroupTest SYSTEM PRIVATE
    ${XWALK_SENSOR_GROUP_GMOCK_INCLUDE_DIRECTORY})
target_include_directories(xWalkSensorGroupTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLineTracker/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkMotor/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLed/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkBuzzer/test/include)
target_compile_features(xWalkSensorGroupTest PRIVATE cxx_std_17)
target_link_libraries(xWalkSensorGroupTest PRIVATE
    GTest::gtest_main
    ${XWALK_SENSOR_GROUP_GMOCK_LIBRARY}
    xWalkBuzzer
    xWalkLed
    xWalkLineTracker
    xWalkMotor)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkSensorGroupTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_test(NAME xWalkSensorGroupTest COMMAND xWalkSensorGroupTest)
set_tests_properties(xWalkSensorGroupTest PROPERTIES
    LABELS "host;sensor-group;group-tests")
