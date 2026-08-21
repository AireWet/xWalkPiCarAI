find_package(GTest CONFIG REQUIRED)
find_library(XWALK_DEVICE_GROUP_GMOCK_LIBRARY NAMES gmock REQUIRED)
find_path(XWALK_DEVICE_GROUP_GMOCK_INCLUDE_DIRECTORY gmock/gmock.h
    HINTS /usr/src/googletest/googlemock/include REQUIRED)

add_executable(xWalkDeviceGroupTest
    src/xHal_Rpi5CarDeviceGroupTest.cpp
    src/xHal_Rpi5CarDeviceGroupTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPwm/test/src/xHal_Rpi5CarPwmTestI2c.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPwm/test/src/xHal_Rpi5CarPwmTestI2cLifecycle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAdc/test/src/xHal_Rpi5CarAdcTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAdxl345/test/src/xHal_Rpi5CarAdxl345TestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkUltrasonic/test/src/xHal_Rpi5CarUltrasonicTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkCamera/test/src/xHal_Rpi5CarCameraTestSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkCamera/hardware/src/xHal_Rpi5CarCameraStreamOpenCv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkUserButton/test/src/xHal_Rpi5CarUserButtonTestSupport.cpp)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs videoio)
target_link_libraries(xWalkDeviceGroupTest PRIVATE xWalkTrace)
target_include_directories(xWalkDeviceGroupTest SYSTEM PRIVATE
    ${XWALK_DEVICE_GROUP_GMOCK_INCLUDE_DIRECTORY})
target_include_directories(xWalkDeviceGroupTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPwm/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAdc/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAdxl345/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkUltrasonic/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkCamera/test/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkCamera/hardware/include
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkUserButton/test/include)
target_compile_features(xWalkDeviceGroupTest PRIVATE cxx_std_17)
target_link_libraries(xWalkDeviceGroupTest PRIVATE
    GTest::gtest_main
    ${XWALK_DEVICE_GROUP_GMOCK_LIBRARY}
    xWalkAdc
    xWalkAdxl345
    xWalkCamera
    ${OpenCV_LIBS}
    xWalkPwm
    xWalkServo
    xWalkUltrasonic
    xWalkUserButton)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkDeviceGroupTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_test(NAME xWalkDeviceGroupTest COMMAND xWalkDeviceGroupTest)
set_tests_properties(xWalkDeviceGroupTest PROPERTIES
    LABELS "host;device-group;group-tests")
