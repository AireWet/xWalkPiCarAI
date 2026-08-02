/******************************************************************************
 * @file        xHal_Rpi5CarRobotTest.cpp
 * @brief       Verifies coordinated xWalk robot behavior using simulated I2C.
 *
 * @details
 * Checks dependency registration, persisted offsets, positioning, interpolation,
 * actions, calibration, reset behavior, and public-boundary validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Test
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarRobot.h"

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains host-test state and scenarios private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records the number of simulated Robot HAT register writes. */
struct TestBus
{
    XWalkHal::uint32 writeCount{}; /**< Number of writes observed by the callback. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Reports that every probed test address is present. */
XWalkHal::boolean probe(XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

/** @brief Records one simulated register write. */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data)
{
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    TestBus& bus = *static_cast<TestBus*>(context);
    ++bus.writeCount;
}

/** @brief Returns zero-filled bytes for the unused read path. */
XWalkHal::bytevector read(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return XWalkHal::bytevector(length, 0U);
}

/**
 * @brief Verifies the complete two-servo robot behavior.
 *
 * @param[in] filePath
 * Test-owned configuration file below the module build directory.
 */
void testRobot(const XWalkHal::filesystempath& filePath)
{
    TestBus bus;
    XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
    XWalkHal::XWalkPwmTimerState timerState;
    XWalkHal::XWalkPwm firstPwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm secondPwm(i2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkServo firstServo(firstPwm);
    XWalkHal::XWalkServo secondServo(secondPwm);
    XWalkHal::XWalkConfigStore store(filePath.string());
    store.set("walker_servo_offset_list", "[5,-5]");

    XWalkHal::XWalkRobot robot(store, "walker", 0U);
    robot.addServo(firstServo, 10.0);
    robot.addServo(secondServo, -10.0);
    robot.initialize({1U, 0U});
    assert(robot.initialized());
    assert(robot.servoCount() == 2U);
    assert(robot.offsets() == XWalkHal::float64vector({5.0, -5.0}));
    assert(robot.servoPositions() == XWalkHal::float64vector({10.0, -10.0}));

    robot.setOriginPositions({1.0, 2.0});
    robot.setDirections({1.0, -1.0});
    robot.setCalibrationPositions({3.0, 4.0});
    robot.calibration();
    assert(robot.servoPositions() == XWalkHal::float64vector({3.0, 4.0}));

    robot.setOffsets({30.0, -30.0});
    assert(robot.offsets() == XWalkHal::float64vector({20.0, -20.0}));
    assert(store.get("walker_servo_offset_list") == "[20.000000,-20.000000]");

    robot.reset({0.0, 0.0});
    robot.servoMove({2.0, -2.0}, 100.0);
    assert(robot.servoPositions() == XWalkHal::float64vector({2.0, -2.0}));
    robot.setAction("wave", {{1.0, -1.0}, {2.0, -2.0}});
    robot.doAction("wave", 1U, 100.0);
    assert(robot.servoPositions() == XWalkHal::float64vector({2.0, -2.0}));

    const XWalkHal::float64vector positionsBeforeSoftReset = robot.servoPositions();
    robot.softReset();
    assert(robot.servoPositions() == positionsBeforeSoftReset);
    robot.reset();
    assert(robot.servoPositions() == XWalkHal::float64vector({0.0, 0.0}));
    assert(bus.writeCount > 0U);
}

/**
 * @brief Verifies rejection of invalid lifecycle and vector inputs.
 *
 * @param[in] filePath
 * Test-owned configuration file below the module build directory.
 */
void testValidation(const XWalkHal::filesystempath& filePath)
{
    TestBus bus;
    XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
    XWalkHal::XWalkPwmTimerState timerState;
    XWalkHal::XWalkPwm pwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkServo servo(pwm);
    XWalkHal::XWalkConfigStore store(filePath.string());
    XWalkHal::XWalkRobot robot(store, "validation", 0U);

    xwalk::hal::test::expectFailure([&]()
    {
        robot.initialize();
    });

    robot.addServo(servo);
    robot.initialize();
    xwalk::hal::test::expectFailure([&]()
    {
        robot.reset({0.0, 1.0});
    });

    xwalk::hal::test::expectFailure([&]()
    {
        robot.doAction("missing");
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all xWalk robot host-test scenarios.
 *
 * @param[in] argumentCount
 * Must equal two so one test path is available.
 *
 * @param[in] arguments
 * Non-owning process argument array containing the test path at index one.
 *
 * @return
 * Zero when every assertion passes; one when the path is absent.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    if (argumentCount != 2)
    {
        return 1;
    }

    const XWalkHal::filesystempath filePath(arguments[1]);
    XWalkHal::filesystempath replacementPath = filePath;
    replacementPath += ".tmp";
    static_cast<void>(XWalkHal::removeFilesystemEntry(filePath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(replacementPath));
    testRobot(filePath);
    static_cast<void>(XWalkHal::removeFilesystemEntry(filePath));
    testValidation(filePath);
    static_cast<void>(XWalkHal::removeFilesystemEntry(filePath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(replacementPath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(filePath.parent_path()));
    return 0;
}
