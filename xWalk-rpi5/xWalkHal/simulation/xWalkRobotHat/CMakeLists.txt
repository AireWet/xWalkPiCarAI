cmake_minimum_required(VERSION 3.16)
project(xWalkRobotHatSimulation LANGUAGES CXX)

add_library(xWalkRobotHatSimulation STATIC
    src/xHal_Rpi5CarRobotHatSimulation.cpp
    src/xHal_Rpi5CarLogicalModels.cpp)
target_link_libraries(xWalkRobotHatSimulation PRIVATE xWalkTrace)
add_library(xWalk::RobotHatSimulation ALIAS xWalkRobotHatSimulation)
target_include_directories(xWalkRobotHatSimulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkRobotHatSimulation PUBLIC cxx_std_17)
target_link_libraries(xWalkRobotHatSimulation PUBLIC xWalkI2c xWalkGpio xWalkCamera)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkRobotHatSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(BUILD_TESTING)
    add_executable(xWalkRobotHatSimulationTest
        test/src/xHal_Rpi5CarRobotHatSimulationTest.cpp
        test/src/xHal_Rpi5CarLogicalModelsTest.cpp
        test/src/xHal_Rpi5CarRobotHatSimulationTestSupport.cpp)
    target_include_directories(xWalkRobotHatSimulationTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_link_libraries(xWalkRobotHatSimulationTest PRIVATE
        xWalkRobotHatSimulation xWalkAdc xWalkPwm xWalkCamera)
    add_test(NAME xWalkRobotHatSimulationHostTest COMMAND xWalkRobotHatSimulationTest)
    set_tests_properties(xWalkRobotHatSimulationHostTest PROPERTIES LABELS "host;simulation;fault-injection")

    add_executable(xWalkRobotHatSoakTest test/src/xHal_Rpi5CarRobotHatSoak.cpp)
    target_include_directories(xWalkRobotHatSoakTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_link_libraries(xWalkRobotHatSoakTest PRIVATE xWalkRobotHatSimulation)
    add_test(NAME xWalkRobotHatSoakHostTest COMMAND xWalkRobotHatSoakTest
        --seed 42 --iterations 5000 --logical-duration 10000 --fault-rate 0.05
        --report ${CMAKE_CURRENT_BINARY_DIR}/xwalk-robot-hat-soak-report.json)
    set_tests_properties(xWalkRobotHatSoakHostTest PROPERTIES
        LABELS "host;simulation;soak;fault-injection" TIMEOUT 30)
endif()
