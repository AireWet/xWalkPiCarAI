/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxTest.cpp
 * @brief       Verifies the PiCar-X agent with in-memory hardware callbacks.
 *
 * @details
 * Covers initialization, steering, motor scaling, calibration persistence, grayscale, cliff, and validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkPicarx Host Test
 *
 * @author      Joxy John
 * @date        2026-07-31
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

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarPicarxSafetyGuard.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains test state and callbacks private to this translation unit. */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Provides deterministic I2C samples and accepts register writes. */
struct TestBus
{
    agent::bytevector sample{0x03U, 0xE8U};
};

/** @brief Stores one simulated GPIO level. */
struct TestGpio
{
    agent::boolean value{};
};

/******************************************************************************
 * Private callback definitions
 ******************************************************************************/

agent::boolean probe(agent::contextpointer context, agent::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

void writeRegister(agent::contextpointer context, agent::uint8 address, agent::uint8 reg,
    const agent::bytevector& data)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
}

/** @brief Accepts one non-throwing simulated fail-safe write. */
agent::boolean tryWriteRegister(agent::contextpointer context, agent::uint8 address,
    agent::uint8 reg, const agent::bytevector& data) noexcept
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    return true;
}

agent::bytevector read(agent::contextpointer context, agent::uint8 address, agent::size length)
{
    static_cast<void>(address);
    static_cast<void>(length);
    return static_cast<TestBus*>(context)->sample;
}

void configureGpio(agent::contextpointer context, agent::uint8 pin, XWalkHal::XWalkGpioMode mode,
    XWalkHal::XWalkGpioPull pull, agent::boolean initialValue)
{
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
    static_cast<TestGpio*>(context)->value = initialValue;
}

agent::boolean readGpio(agent::contextpointer context, agent::uint8 pin)
{
    static_cast<void>(pin);
    return static_cast<TestGpio*>(context)->value;
}

void writeGpio(agent::contextpointer context, agent::uint8 pin, agent::boolean value)
{
    static_cast<void>(pin);
    static_cast<TestGpio*>(context)->value = value;
}

void interruptGpio(agent::contextpointer context, agent::uint8 pin, XWalkHal::XWalkGpioEdge edge,
    agent::uint32 debounceMs, agent::contextpointer handlerContext,
    XWalkHal::gpiointerrupthandler handler)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(edge);
    static_cast<void>(debounceMs);
    static_cast<void>(handlerContext);
    static_cast<void>(handler);
}

void cancelInterrupt(agent::contextpointer context, agent::uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

XWalkHal::XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelInterrupt};
}

/******************************************************************************
 * Test function definitions
 ******************************************************************************/

/**
 * @brief Exercises the port through a complete in-memory HAL composition.
 *
 * @param[in] configPath
 * Test-owned configuration path below the module build directory.
 */
void testPicarxBehavior(agent::stringview configPath)
{
    TestBus bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr,
        &tryWriteRegister);
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm leftPwm(i2c, "P13", 0x14U, timerState);
    xwalk::hal::XWalkPwm rightPwm(i2c, "P12", 0x14U, timerState);
    xwalk::hal::XWalkPwm directionPwm(i2c, "P2", 0x14U, timerState);
    xwalk::hal::XWalkPwm panPwm(i2c, "P0", 0x14U, timerState);
    xwalk::hal::XWalkPwm tiltPwm(i2c, "P1", 0x14U, timerState);
    TestGpio leftBackend;
    TestGpio rightBackend;
    TestGpio triggerBackend;
    TestGpio echoBackend;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    xwalk::hal::XWalkGpio leftDirection(&leftBackend, callbacks, "D4");
    xwalk::hal::XWalkGpio rightDirection(&rightBackend, callbacks, "D5");
    xwalk::hal::XWalkGpio trigger(&triggerBackend, callbacks, "D2");
    xwalk::hal::XWalkGpio echo(&echoBackend, callbacks, "D3");
    xwalk::hal::XWalkMotor leftMotor(leftPwm, leftDirection);
    xwalk::hal::XWalkMotor rightMotor(rightPwm, rightDirection);
    xwalk::hal::XWalkMotors motors(leftMotor, rightMotor);
    xwalk::hal::XWalkServo directionServo(directionPwm);
    xwalk::hal::XWalkServo panServo(panPwm);
    xwalk::hal::XWalkServo tiltServo(tiltPwm);
    xwalk::hal::XWalkAdc adc0(i2c, "A0", 0x14U);
    xwalk::hal::XWalkAdc adc1(i2c, "A1", 0x14U);
    xwalk::hal::XWalkAdc adc2(i2c, "A2", 0x14U);
    xwalk::hal::XWalkGrayscaleModule grayscale(adc0, adc1, adc2);
    xwalk::hal::XWalkUltrasonic ultrasonic(trigger, echo, 0U);
    xwalk::hal::XWalkConfigStore config(configPath);
    config.set("picarx_dir_servo", "5");
    config.set("picarx_max_motor_output_percent", "100");
    config.set("picarx_calibration_verified", "true");
    config.set("line_reference", "[900, 1000, 1100]");

    xwalk::agent::XWalkPicarx picarx(motors, directionServo, panServo, tiltServo,
        grayscale, ultrasonic, config);
    assert(picarx.isInitialized() == false);
    assert(picarx.initialize());
    assert(picarx.initialize() == false);
    assert(picarx.maximumMotorOutputPercent() == 100.0);
    picarx.setDirectionServoAngle(30.0);
    assert(picarx.directionAngleDegrees() == 30.0);
    picarx.forward(40.0);
    assert(motors.left().speed() == 64.0);
    assert(motors.right().speed() == 70.0);

    picarx.calibrateMotorSpeed(10.0);
    assert(config.get("picarx_motor_speed_calibration") == "10.000000");
    picarx.setPower(20.0);
    assert(motors.left().speed() == 50.0);
    assert(motors.right().speed() == 60.0);
    picarx.calibrateMotorDirection(2U, -1);
    assert(motors.configuration().rightReversed);
    assert(config.get("picarx_dir_motor") == "[1,-1]");

    picarx.clearEmergencyStop();
    picarx.forward(40.0);
    {
        xwalk::agent::XWalkPicarxSafetyGuard safetyGuard(picarx);
    }
    assert(picarx.emergencyStopRequested());
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);
    picarx.forward(40.0);
    picarx.setDirectionServoAngle(-20.0);
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);
    assert(picarx.directionAngleDegrees() == 30.0);
    picarx.clearEmergencyStop();

    const XWalkHal::linetrackervalues readings = picarx.grayscaleData();
    assert(readings == XWalkHal::linetrackervalues({1000, 1000, 1000}));
    const XWalkHal::linetrackerstatus status = picarx.lineStatus(readings);
    assert(status == XWalkHal::linetrackerstatus({0U, 1U, 1U}));
    assert(picarx.cliffStatus({500, 501, 502}));
    picarx.setCliffReference({400, 401, 402});
    assert(!picarx.cliffStatus({500, 501, 502}));
    picarx.close();
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);

    xwalk::agent::XWalkPicarx reloadedPicarx(motors, directionServo, panServo,
        tiltServo, grayscale, ultrasonic, config);
    static_cast<void>(reloadedPicarx.initialize());
    reloadedPicarx.setPower(20.0);
    assert(motors.left().speed() == 50.0);
    assert(motors.right().speed() == 60.0);
    reloadedPicarx.close();

    config.set("picarx_motor_speed_calibration", "invalid");
    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::agent::XWalkPicarx invalidPicarx(motors, directionServo, panServo,
            tiltServo, grayscale, ultrasonic, config);
    });
    config.set("picarx_motor_speed_calibration", "101");
    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::agent::XWalkPicarx invalidPicarx(motors, directionServo, panServo,
            tiltServo, grayscale, ultrasonic, config);
    });
    config.set("picarx_motor_speed_calibration", "0");
    struct InvalidConfiguration
    {
        agent::cstring key;
        agent::cstring invalidValue;
        agent::cstring validValue;
    };
    const agent::fixedarray<InvalidConfiguration, 16U> invalidConfigurations{{
        {"picarx_dir_servo", "nan", "5"},
        {"picarx_max_motor_output_percent", "101", "100"},
        {"picarx_calibration_verified", "yes", "true"},
        {"picarx_apply_persisted_servo_positions", "yes", "false"},
        {"picarx_dir_motor", "1,1", "[1, 1]"},
        {"picarx_dir_motor", "[0,1]", "[1, 1]"},
        {"picarx_dir_motor", "[1 1]", "[1, 1]"},
        {"picarx_dir_motor", "[1,1]x", "[1, 1]"},
        {"line_reference", "1,2,3", "[900, 1000, 1100]"},
        {"line_reference", "[1.5,2,3]", "[900, 1000, 1100]"},
        {"line_reference", "[1,2]", "[900, 1000, 1100]"},
        {"line_reference", "[1,2,3,4]", "[900, 1000, 1100]"},
        {"line_reference", "[1, x, 3]", "[900, 1000, 1100]"},
        {"cliff_reference", "[1,2,2147483648]", "[500, 500, 500]"},
        {"cliff_reference", "[1,2,-2147483649]", "[500, 500, 500]"},
        {"cliff_reference", "[1,2,3] trailing", "[500, 500, 500]"}
    }};
    for (const InvalidConfiguration& invalidConfiguration : invalidConfigurations)
    {
        config.set(invalidConfiguration.key, invalidConfiguration.invalidValue);
        xwalk::hal::test::expectFailure([&]()
        {
            xwalk::agent::XWalkPicarx invalidPicarx(motors, directionServo,
                panServo, tiltServo, grayscale, ultrasonic, config);
        });
        config.set(invalidConfiguration.key, invalidConfiguration.validValue);
    }
}

/**
 * @brief Verifies the deployment-safe default motor-output limit.
 * @param[in] configPath Fresh test-owned configuration path below the module build directory.
 */
void testFirstRunOutputLimit(agent::stringview configPath)
{
    TestBus bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr,
        &tryWriteRegister);
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm leftPwm(i2c, "P13", 0x14U, timerState);
    xwalk::hal::XWalkPwm rightPwm(i2c, "P12", 0x14U, timerState);
    xwalk::hal::XWalkPwm directionPwm(i2c, "P2", 0x14U, timerState);
    xwalk::hal::XWalkPwm panPwm(i2c, "P0", 0x14U, timerState);
    xwalk::hal::XWalkPwm tiltPwm(i2c, "P1", 0x14U, timerState);
    TestGpio leftBackend;
    TestGpio rightBackend;
    TestGpio triggerBackend;
    TestGpio echoBackend;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    xwalk::hal::XWalkGpio leftDirection(&leftBackend, callbacks, "D4");
    xwalk::hal::XWalkGpio rightDirection(&rightBackend, callbacks, "D5");
    xwalk::hal::XWalkGpio trigger(&triggerBackend, callbacks, "D2");
    xwalk::hal::XWalkGpio echo(&echoBackend, callbacks, "D3");
    xwalk::hal::XWalkMotor leftMotor(leftPwm, leftDirection);
    xwalk::hal::XWalkMotor rightMotor(rightPwm, rightDirection);
    xwalk::hal::XWalkMotors motors(leftMotor, rightMotor);
    xwalk::hal::XWalkServo directionServo(directionPwm);
    xwalk::hal::XWalkServo panServo(panPwm);
    xwalk::hal::XWalkServo tiltServo(tiltPwm);
    xwalk::hal::XWalkAdc adc0(i2c, "A0", 0x14U);
    xwalk::hal::XWalkAdc adc1(i2c, "A1", 0x14U);
    xwalk::hal::XWalkAdc adc2(i2c, "A2", 0x14U);
    xwalk::hal::XWalkGrayscaleModule grayscale(adc0, adc1, adc2);
    xwalk::hal::XWalkUltrasonic ultrasonic(trigger, echo, 0U);
    xwalk::hal::XWalkConfigStore config(configPath);
    config.set("picarx_max_motor_output_percent", "100");
    xwalk::agent::XWalkPicarx picarx(motors, directionServo, panServo, tiltServo,
        grayscale, ultrasonic, config);
    static_cast<void>(picarx.initialize());

    assert(!picarx.calibrationVerified());
    assert(picarx.maximumMotorOutputPercent() == 20.0);
    picarx.forward(100.0);
    assert(motors.left().speed() == 20.0);
    assert(motors.right().speed() == 20.0);
    picarx.backward(100.0);
    assert(motors.left().speed() == -20.0);
    assert(motors.right().speed() == -20.0);
    picarx.stop();
    picarx.recordCalibrationVerified(true);
    assert(picarx.calibrationVerified());
    assert(config.get("picarx_calibration_verified") == "true");
    assert(picarx.maximumMotorOutputPercent() == 100.0);
    picarx.forward(100.0);
    assert(motors.left().speed() == 100.0);
    assert(motors.right().speed() == 100.0);
    picarx.stop();
    picarx.recordCalibrationVerified(false);
    assert(picarx.maximumMotorOutputPercent() == 20.0);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all PiCar-X host-test scenarios.
 *
 * @param[in] argumentCount
 * Must equal two so one test-owned configuration path is available.
 *
 * @param[in] arguments
 * Non-owning process argument array whose second entry is the configuration path.
 *
 * @return
 * Zero when every assertion passes; one when the required path is absent.
 */
agent::int32 main(agent::int32 argumentCount, agent::charpointer arguments[])
{
    if (argumentCount != 2)
    {
        return 1;
    }

    const agent::filesystempath configPath(arguments[1]);
    agent::filesystempath replacementPath = configPath;
    replacementPath += ".tmp";
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    testPicarxBehavior(configPath.string());
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    testFirstRunOutputLimit(configPath.string());
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath.parent_path()));
    return 0;
}
