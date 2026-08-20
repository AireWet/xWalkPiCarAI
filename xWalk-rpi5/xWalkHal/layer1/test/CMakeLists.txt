find_package(GTest CONFIG REQUIRED)
find_library(XWALK_LAYER1_GROUP_GMOCK_LIBRARY NAMES gmock REQUIRED)
find_path(XWALK_LAYER1_GROUP_GMOCK_INCLUDE_DIRECTORY gmock/gmock.h
    HINTS /usr/src/googletest/googlemock/include REQUIRED)
string(CONCAT XWALK_LAYER1_GROUP_LANGUAGE_MODEL_TEST_SUPPORT
    "${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkLanguageModel/test/src/"
    "xHal_Rpi5CarLanguageModelTestSupport.cpp")

add_executable(xWalkLayer1GroupTest
    src/xHal_Rpi5CarLayer1GroupTest.cpp
    src/xHal_Rpi5CarLayer1GroupTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkMusic/test/src/xHal_Rpi5CarMusicTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkSpeaker/test/src/xHal_Rpi5CarSpeakerTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkBoardControl/test/src/xHal_Rpi5CarBoardControlTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkRobot/test/src/xHal_Rpi5CarRobotTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGPT/test/src/xHal_Rpi5CarGptTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkVoiceAssistant/test/src/xHal_Rpi5CarVoiceAssistantTestSupport.cpp
    ${XWALK_LAYER1_GROUP_LANGUAGE_MODEL_TEST_SUPPORT})
target_link_libraries(xWalkLayer1GroupTest PRIVATE xWalkTrace)
target_include_directories(xWalkLayer1GroupTest SYSTEM PRIVATE
    ${XWALK_LAYER1_GROUP_GMOCK_INCLUDE_DIRECTORY})
target_include_directories(xWalkLayer1GroupTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkMusic/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkSpeaker/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkBoardControl/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkRobot/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGPT/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGPT/hardware/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkVoiceAssistant/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkLanguageModel/test/include)
target_compile_features(xWalkLayer1GroupTest PRIVATE cxx_std_17)
target_compile_definitions(xWalkLayer1GroupTest PRIVATE
    XWALK_LAYER1_GROUP_TEST_DATA_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/test-data")
target_link_libraries(xWalkLayer1GroupTest PRIVATE
    GTest::gtest_main
    ${XWALK_LAYER1_GROUP_GMOCK_LIBRARY}
    xWalkBoardControl
    xWalkGPT
    xWalkLanguageModel
    xWalkMusic
    xWalkRobot
    xWalkSpeaker
    xWalkVoiceAssistant)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLayer1GroupTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_test(NAME xWalkLayer1GroupTest COMMAND xWalkLayer1GroupTest)
set_tests_properties(xWalkLayer1GroupTest PROPERTIES
    LABELS "host;layer1-group;group-tests")
