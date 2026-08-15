/******************************************************************************
 * @file        xHal_Rpi5CarMotorTest.cpp
 * @brief       Verifies motor behavior using in-memory callbacks.
 * @project     xWalk Firmware
 * @module      xWalkMotor Host Test
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarMotor.h"
#include "xHal_Rpi5CarMotorSimulationArguments.h"
#include "xHal_Rpi5CarMotorSimulationConfig.h"
#include "xHal_Rpi5CarMotorTestSupport.h"
#include "xHal_Rpi5CarMotors.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include <limits>
namespace
{
    using namespace xwalk::hal::test::motor;

    /** @brief Verifies PWM-and-direction speed, stop, and reversal behavior. */
    void testPwmAndDirectionMode()
    {
        TestI2c bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm pwm(i2c, 13U, 0x14U, timerState);
        TestGpio gpioBackend;
        const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
        xwalk::hal::XWalkGpio direction(&gpioBackend, callbacks, "D4");
        const XWalkHal::uint32 writesBeforeMotorConstruction = bus.writeCount;
        xwalk::hal::XWalkMotor motor(pwm, direction);
        assert(bus.writeCount == writesBeforeMotorConstruction);
        assert(!motor.initialized());
        xwalk::hal::test::expectFailure(
            [&motor]()
            {
                motor.setSpeed(1.0);
            });
        assert(bus.writeCount == writesBeforeMotorConstruction);
        assert(motor.stopSafely());
        assert(bus.writeCount == writesBeforeMotorConstruction);
        assert(motor.initialize());
        const XWalkHal::uint32 initializationWrites = bus.writeCount;
        assert(motor.initialize());
        assert(bus.writeCount == initializationWrites);
        assert(motor.initialized());
        assert(motor.mode() == XWalkHal::XWalkMotorMode::PwmAndDirection);
        assert(motor.frequency() == 100.0);
        motor.setSpeed(50.0);
        assert((motor.speed() == 50.0) && (pwm.pulseWidthPercent() == 50.0));
        assert(gpioBackend.value);
        motor.setSpeed(-25.0);
        assert((motor.speed() == -25.0) && (pwm.pulseWidthPercent() == 25.0));
        assert(gpioBackend.value == false);
        motor.setReversed(true);
        motor.setSpeed(10.0);
        assert(gpioBackend.value == false);
        motor.stop();
        assert((motor.speed() == 0.0) && (pwm.pulseWidthPercent() == 0.0));
    }

    /** @brief Verifies dual-PWM speed selection, reversal, stopping, and braking.
     */
    void testDualPwmMode()
    {
        TestI2c bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm forwardPwm(i2c, 12U, 0x14U, timerState);
        xwalk::hal::XWalkPwm reversePwm(i2c, 13U, 0x14U, timerState);
        const XWalkHal::uint32 writesBeforeMotorConstruction = bus.writeCount;
        xwalk::hal::XWalkMotor motor(forwardPwm, reversePwm);
        assert(bus.writeCount == writesBeforeMotorConstruction);
        xwalk::hal::test::expectFailure(
            [&motor]()
            {
                motor.brake();
            });
        assert(bus.writeCount == writesBeforeMotorConstruction);
        assert(motor.initialize());
        motor.setSpeed(40.0);
        assert((forwardPwm.pulseWidthPercent() == 40.0) && (reversePwm.pulseWidthPercent() == 0.0));
        motor.setSpeed(-30.0);
        assert((forwardPwm.pulseWidthPercent() == 0.0) && (reversePwm.pulseWidthPercent() == 30.0));
        motor.setReversed(true);
        motor.setSpeed(20.0);
        assert(reversePwm.pulseWidthPercent() == 20.0);
        motor.brake();
        assert((forwardPwm.pulseWidthPercent() == 100.0) && (reversePwm.pulseWidthPercent() == 100.0));
        assert(motor.speed() == 0.0);
    }

    /** @brief Verifies paired movement, role configuration, and fail-safe stopping.
     */
    void testPairedMotors()
    {
        TestI2c bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm firstForward(i2c, 12U, 0x14U, timerState);
        xwalk::hal::XWalkPwm firstReverse(i2c, 13U, 0x14U, timerState);
        xwalk::hal::XWalkPwm secondForward(i2c, 14U, 0x14U, timerState);
        xwalk::hal::XWalkPwm secondReverse(i2c, 15U, 0x14U, timerState);
        xwalk::hal::XWalkMotor firstMotor(firstForward, firstReverse);
        xwalk::hal::XWalkMotor secondMotor(secondForward, secondReverse);
        xwalk::hal::XWalkMotorsConfiguration configuration;
        configuration.watchdogWorkerEnabled = false;
        xwalk::hal::XWalkMotors motors(firstMotor, secondMotor, configuration);
        motors.arm();
        motors.turnLeft(35.0);
        assert((motors.left().speed() == -35.0) && (motors.right().speed() == 35.0));
        motors.turnRight(20.0);
        assert((motors.left().speed() == 20.0) && (motors.right().speed() == -20.0));
        motors.setLeftMotorId(2U);
        motors.setRightMotorId(1U);
        motors.forward(10.0);
        assert((secondMotor.speed() == 10.0) && (firstMotor.speed() == 10.0));
        assert(motors.toggleLeftReversed());
        assert(secondMotor.reversed());
        motors.setSpeed(30.0, 40.0);
        const XWalkHal::uint32 firstStopWrite = bus.writeCount + 1U;
        bus.failingWrites = {firstStopWrite, firstStopWrite + 2U};
        assert(motors.stopSafely() == false);
        assert(bus.writeCount == (firstStopWrite + 3U));
        bus.failingWrites.clear();
        assert(motors.stopSafely());
    }

    /** @brief Verifies speed, frequency, identifier, and mode validation. */
    void testValidation()
    {
        TestI2c bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm pwm(i2c, 13U, 0x14U, timerState);
        xwalk::hal::XWalkPwm secondPwm(i2c, 12U, 0x14U, timerState);
        TestGpio firstGpioBackend;
        TestGpio secondGpioBackend;
        const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
        xwalk::hal::XWalkGpio direction(&firstGpioBackend, callbacks, "D4");
        xwalk::hal::XWalkGpio secondDirection(&secondGpioBackend, callbacks, "D5");
        xwalk::hal::XWalkMotor motor(pwm, direction);
        xwalk::hal::XWalkMotor secondMotor(secondPwm, secondDirection);
        xwalk::hal::XWalkMotorsConfiguration configuration;
        configuration.watchdogWorkerEnabled = false;
        xwalk::hal::XWalkMotors motors(motor, secondMotor, configuration);
        motors.arm();
        xwalk::hal::test::expectFailure(
            [&]()
            {
                motor.setSpeed(101.0);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                motor.setSpeed(XHAL_POSITIVE_INFINITY(XWalkHal::float64));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                motor.brake();
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::XWalkMotor invalidFrequencyMotor(pwm, direction, false, 0.0);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                motors.setLeftMotorId(0U);
            });
        motors.setSpeed(12.0, 13.0);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                motors.setSpeed(20.0, 101.0);
            });
        assert((motor.speed() == 12.0) && (secondMotor.speed() == 13.0));
    }

    /** @brief Verifies exact deterministic watchdog expiry and explicit recovery.
     */
    void testDeterministicWatchdog()
    {
        TestI2c bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm firstForward(i2c, 12U, 0x14U, timerState);
        xwalk::hal::XWalkPwm firstReverse(i2c, 13U, 0x14U, timerState);
        xwalk::hal::XWalkPwm secondForward(i2c, 14U, 0x14U, timerState);
        xwalk::hal::XWalkPwm secondReverse(i2c, 15U, 0x14U, timerState);
        xwalk::hal::XWalkMotor firstMotor(firstForward, firstReverse);
        xwalk::hal::XWalkMotor secondMotor(secondForward, secondReverse);
        FakeClock clock;
        xwalk::hal::XWalkMotorsConfiguration configuration;
        configuration.watchdogTimeoutMilliseconds = 100U;
        configuration.clockContext = &clock;
        configuration.clockMilliseconds = &clockMilliseconds;
        configuration.watchdogWorkerEnabled = false;
        xwalk::hal::XWalkMotors motors(firstMotor, secondMotor, configuration);

        motors.arm();
        motors.forward(25.0);
        clock.milliseconds = 99U;
        assert(!motors.checkWatchdog());
        xwalk::hal::test::expectFailure(
            [&motors]()
            {
                motors.setSpeed(101.0, 0.0);
            });
        clock.milliseconds = 100U;
        assert(motors.checkWatchdog());
        assert(!motors.isArmed());
        assert((firstMotor.speed() == 0.0) && (secondMotor.speed() == 0.0));
        xwalk::hal::test::expectFailure(
            [&motors]()
            {
                motors.forward(10.0);
            });

        motors.arm();
        clock.milliseconds = 150U;
        motors.forward(20.0);
        clock.milliseconds = 249U;
        motors.heartbeat();
        clock.milliseconds = 348U;
        assert(!motors.checkWatchdog());
        clock.milliseconds = 349U;
        assert(motors.checkWatchdog());
        assert(motors.disarm());
        assert(motors.disarm());
    }

    /** @brief Verifies clock rollback and large forward jumps expire safely. */
    void testWatchdogClockDiscontinuities()
    {
        TestI2c bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm firstForward(i2c, 12U, 0x14U, timerState);
        xwalk::hal::XWalkPwm firstReverse(i2c, 13U, 0x14U, timerState);
        xwalk::hal::XWalkPwm secondForward(i2c, 14U, 0x14U, timerState);
        xwalk::hal::XWalkPwm secondReverse(i2c, 15U, 0x14U, timerState);
        xwalk::hal::XWalkMotor firstMotor(firstForward, firstReverse);
        xwalk::hal::XWalkMotor secondMotor(secondForward, secondReverse);
        FakeClock clock;
        clock.milliseconds = 1'000U;
        xwalk::hal::XWalkMotorsConfiguration configuration;
        configuration.watchdogTimeoutMilliseconds = 500U;
        configuration.clockContext = &clock;
        configuration.clockMilliseconds = &clockMilliseconds;
        configuration.watchdogWorkerEnabled = false;
        xwalk::hal::XWalkMotors motors(firstMotor, secondMotor, configuration);

        motors.arm();
        motors.forward(20.0);
        clock.milliseconds = 999U;
        assert(motors.checkWatchdog());
        assert(!motors.isArmed());
        assert((firstMotor.speed() == 0.0) && (secondMotor.speed() == 0.0));

        clock.milliseconds = 2'000U;
        motors.arm();
        motors.forward(20.0);
        clock.milliseconds = std::numeric_limits<XWalkHal::uint64>::max();
        assert(motors.checkWatchdog());
        assert(!motors.isArmed());
        assert((firstMotor.speed() == 0.0) && (secondMotor.speed() == 0.0));
    }

    /** @brief Verifies watchdog timeout boundaries without waiting in real time. */
    void testWatchdogConfigurationBounds()
    {
        TestI2c bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm firstPwm(i2c, 12U, 0x14U, timerState);
        xwalk::hal::XWalkPwm secondPwm(i2c, 13U, 0x14U, timerState);
        TestGpio firstBackend;
        TestGpio secondBackend;
        xwalk::hal::XWalkGpio firstDirection(&firstBackend, gpioCallbacks(), "D4");
        xwalk::hal::XWalkGpio secondDirection(&secondBackend, gpioCallbacks(), "D5");
        xwalk::hal::XWalkMotor firstMotor(firstPwm, firstDirection);
        xwalk::hal::XWalkMotor secondMotor(secondPwm, secondDirection);
        xwalk::hal::XWalkMotorsConfiguration configuration;
        configuration.watchdogWorkerEnabled = false;
        configuration.watchdogTimeoutMilliseconds = 1U;
        xwalk::hal::XWalkMotors minimum(firstMotor, secondMotor, configuration);
        static_cast<void>(minimum.disarm());
        configuration.watchdogTimeoutMilliseconds = xwalk::hal::XHAL_RPI5CAR_MOTOR_WATCHDOG_MAXIMUM_MILLISECONDS;
        xwalk::hal::XWalkMotors maximum(firstMotor, secondMotor, configuration);
        static_cast<void>(maximum.disarm());
        configuration.watchdogTimeoutMilliseconds = 0U;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::XWalkMotors invalid(firstMotor, secondMotor, configuration);
            });
        configuration.watchdogTimeoutMilliseconds = xwalk::hal::XHAL_RPI5CAR_MOTOR_WATCHDOG_MAXIMUM_MILLISECONDS + 1U;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::XWalkMotors invalid(firstMotor, secondMotor, configuration);
            });
    }

    /** @brief Verifies a watchdog-worker startup failure performs no motor
     * operation. */
    void testWatchdogThreadStartFailure()
    {
        TestI2c bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm firstPwm(i2c, 12U, 0x14U, timerState);
        xwalk::hal::XWalkPwm secondPwm(i2c, 13U, 0x14U, timerState);
        TestGpio firstBackend;
        TestGpio secondBackend;
        xwalk::hal::XWalkGpio firstDirection(&firstBackend, gpioCallbacks(), "D4");
        xwalk::hal::XWalkGpio secondDirection(&secondBackend, gpioCallbacks(), "D5");
        xwalk::hal::XWalkMotor firstMotor(firstPwm, firstDirection);
        xwalk::hal::XWalkMotor secondMotor(secondPwm, secondDirection);
        const XWalkHal::uint32 writesBeforeConstruction = bus.writeCount;
        xwalk::hal::XWalkMotorsConfiguration configuration;
        configuration.beforeThreadStart = &failThreadStart;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::XWalkMotors motors(firstMotor, secondMotor, configuration);
                static_cast<void>(motors);
            });
        assert(bus.writeCount == writesBeforeConstruction);
        assert(!firstMotor.initialized() && !secondMotor.initialized());
    }

    /** @brief Verifies partial single-motor initialization remains uninitialized
     * and recoverable. */
    void testPartialInitializationFailure()
    {
        TestI2c bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm forwardPwm(i2c, 12U, 0x14U, timerState);
        xwalk::hal::XWalkPwm reversePwm(i2c, 13U, 0x14U, timerState);
        xwalk::hal::XWalkMotor motor(forwardPwm, reversePwm);
        bus.failingWrites = {bus.writeCount + 1U};
        xwalk::hal::test::expectFailure(
            [&motor]()
            {
                static_cast<void>(motor.initialize());
            });
        assert(!motor.initialized());
        assert(motor.speed() == 0.0);
        bus.failingWrites.clear();
        assert(motor.initialize());
        assert(motor.initialized());
    }

    /** @brief Verifies persistent Motor trace-selector behavior. */
    void testTraceSelection()
    {
        char executable[] = "xWalkMotorTest";
        char option[] = "--trace";
        char enableSelector[] = "RPI.253.enable";
        char disableSelector[] = "RPI.253.disable";
        char malformedSelector[] = "RPI.invalid.enable";
        XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
        xwalk::hal::sim::XWalkMotorSimulationArguments enable(3, enableArguments);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());
        XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
        xwalk::hal::sim::XWalkMotorSimulationArguments disable(3, disableArguments);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());
        XWalkHal::charpointer malformedArguments[]{executable, option, malformedSelector};
        const xwalk::hal::sim::XWalkMotorSimulationArguments malformed(3, malformedArguments);
        assert(malformed.valid() == false);
    }
} /* namespace */

/** @brief Runs all xWalkMotor host-test scenarios. */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_MOTOR_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_MOTOR_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .256, "xWalkMotor host tests started");
    testPwmAndDirectionMode();
    testDualPwmMode();
    testPairedMotors();
    testValidation();
    testDeterministicWatchdog();
    testWatchdogClockDiscontinuities();
    testWatchdogConfigurationBounds();
    testWatchdogThreadStartFailure();
    testPartialInitializationFailure();
    testTraceSelection();
    XWALK_HAL_TRACE_UID0(RPI .257, "xWalkMotor host tests completed");
    return 0;
}
