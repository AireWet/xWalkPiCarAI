/******************************************************************************
 * @file        xHal_Rpi5CarSensorGroupTest.cpp
 * @brief       Verifies collaboration among sensor-group modules.
 *
 * @details
 * Uses a test-only line-response policy to connect real LineTracker, Motor,
 * LED, and Buzzer contracts over deterministic lower-level callback fakes.
 *
 * @project     xWalk Firmware
 * @module      xWalk Sensor Group Test
 *
 * @author      Joxy John
 * @date        2026-08-10
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

#include "xHal_Rpi5CarBuzzer.h"
#include "xHal_Rpi5CarBuzzerTestSupport.h"
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarLedTestSupport.h"
#include "xHal_Rpi5CarLineTracker.h"
#include "xHal_Rpi5CarLineTrackerTestSupport.h"
#include "xHal_Rpi5CarMotor.h"
#include "xHal_Rpi5CarMotors.h"
#include "xHal_Rpi5CarMotorTestSupport.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "xHal_Rpi5CarSensorGroupTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using LineResponseCase = ::xwalk::source_types::xhal_rpi5carsensorgrouptest::LineResponseCase;
using LineResponseGroupTest = ::xwalk::source_types::xhal_rpi5carsensorgrouptest::LineResponseGroupTest;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains sensor-group scenarios private to this translation unit. */
namespace
{

    using namespace xwalk::hal;

    /**
     * @brief Applies the group-level line-response policy through public contracts.
     * @param[in,out] tracker Real tracker used to classify the supplied values.
     * @param[in] values Complete left, middle, and right ADC values.
     * @param[in,out] motors Paired motor abstraction receiving movement or stop commands.
     * @param[in,out] statusLed LED receiving normal or critical indication.
     * @param[in,out] buzzer Active buzzer receiving critical alarm state.
     */
    void applyLineResponse(XWalkLineTracker& tracker,
                           const linetrackervalues& values,
                           XWalkMotors& motors,
                           XWalkLed& statusLed,
                           XWalkBuzzer& buzzer)
    {
        const boolean linePresent = tracker.isOnLine(values);
        if (linePresent == false)
        {
            motors.stop();
            statusLed.off();
            buzzer.on();
            return;
        }

        buzzer.off();
        statusLed.on();
        const float64 position = tracker.getLinePosition(values);
        const boolean lineIsLeft = position < -0.1;
        const boolean lineIsRight = position > 0.1;
        if (lineIsLeft)
        {
            motors.turnLeft(35.0);
        }
        else if (lineIsRight)
        {
            motors.turnRight(35.0);
        }
        else
        {
            motors.forward(35.0);
        }
    }

    /** @brief Verifies centred, left, right, and lost-line motor and status responses. */
    TEST_P(LineResponseGroupTest, TrackerObservationControlsMotorsLedAndBuzzer)
    {
        using namespace xwalk::hal::test;
        const LineResponseCase responseCase = GetParam();

        linetracker::TestBus trackerBus;
        trackerBus.samples = responseCase.samples;
        XWalkI2c trackerI2c(&trackerBus, &linetracker::probe, &linetracker::writeRegister, &linetracker::read);
        XWalkAdc leftAdc(trackerI2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkAdc middleAdc(trackerI2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkAdc rightAdc(trackerI2c, 2U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkLineTracker tracker(leftAdc, middleAdc, rightAdc);

        motor::TestI2c motorBus;
        XWalkI2c motorI2c(
            &motorBus, &motor::probe, &motor::writeRegister, &motor::read, nullptr, &motor::tryWriteRegister);
        XWalkPwmTimerState timerState;
        XWalkPwm leftPwm(motorI2c, 12U, 0x14U, timerState);
        XWalkPwm rightPwm(motorI2c, 13U, 0x14U, timerState);
        motor::TestGpio leftDirectionBackend;
        motor::TestGpio rightDirectionBackend;
        XWalkGpio leftDirection(&leftDirectionBackend, motor::gpioCallbacks(), "D4");
        XWalkGpio rightDirection(&rightDirectionBackend, motor::gpioCallbacks(), "D5");
        XWalkMotor leftMotor(leftPwm, leftDirection);
        XWalkMotor rightMotor(rightPwm, rightDirection);
        XWalkMotors motors(leftMotor, rightMotor);
        motors.arm();

        led::GpioBackend ledBackend;
        XWalkGpio ledGpio(&ledBackend, led::gpioCallbacks(), "LED");
        XWalkLed statusLed(ledGpio);
        buzzer::TestBackend buzzerBackend;
        XWalkGpio buzzerGpio(&buzzerBackend, buzzer::gpioCallbacks(), "D2");
        XWalkBuzzer buzzer(buzzerGpio);

        const linetrackervalues values = tracker.read(true);
        EXPECT_THAT(values,
                    testing::ElementsAre(responseCase.samples[0U], responseCase.samples[1U], responseCase.samples[2U]));
        applyLineResponse(tracker, values, motors, statusLed, buzzer);

        EXPECT_DOUBLE_EQ(motors.left().speed(), responseCase.leftSpeedPercent);
        EXPECT_DOUBLE_EQ(motors.right().speed(), responseCase.rightSpeedPercent);
        EXPECT_EQ(statusLed.isOn(), responseCase.critical == false);
        EXPECT_EQ(buzzer.isOn(), responseCase.critical);
        EXPECT_EQ(ledBackend.value, responseCase.critical == false);
        EXPECT_EQ(buzzerBackend.gpioValue, responseCase.critical);
    }

    INSTANTIATE_TEST_SUITE_P(LinePositions,
                             LineResponseGroupTest,
                             testing::Values(LineResponseCase{{1'000U, 200U, 1'000U}, 35.0, 35.0, false},
                                             LineResponseCase{{200U, 1'000U, 1'000U}, -35.0, 35.0, false},
                                             LineResponseCase{{1'000U, 1'000U, 200U}, 35.0, -35.0, false},
                                             LineResponseCase{{1'000U, 1'000U, 1'000U}, 0.0, 0.0, true}));

    /** @brief Verifies recovery from a critical indication restores every actuator. */
    TEST(XWalkSensorGroup, SafeRecoveryStopsAlarmAndRestoresForwardMotion)
    {
        using namespace xwalk::hal::test;
        linetracker::TestBus trackerBus;
        XWalkI2c trackerI2c(&trackerBus, &linetracker::probe, &linetracker::writeRegister, &linetracker::read);
        XWalkAdc leftAdc(trackerI2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkAdc middleAdc(trackerI2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkAdc rightAdc(trackerI2c, 2U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkLineTracker tracker(leftAdc, middleAdc, rightAdc);

        motor::TestI2c motorBus;
        XWalkI2c motorI2c(
            &motorBus, &motor::probe, &motor::writeRegister, &motor::read, nullptr, &motor::tryWriteRegister);
        XWalkPwmTimerState timerState;
        XWalkPwm leftPwm(motorI2c, 12U, 0x14U, timerState);
        XWalkPwm rightPwm(motorI2c, 13U, 0x14U, timerState);
        motor::TestGpio leftDirectionBackend;
        motor::TestGpio rightDirectionBackend;
        XWalkGpio leftDirection(&leftDirectionBackend, motor::gpioCallbacks(), "D4");
        XWalkGpio rightDirection(&rightDirectionBackend, motor::gpioCallbacks(), "D5");
        XWalkMotor leftMotor(leftPwm, leftDirection);
        XWalkMotor rightMotor(rightPwm, rightDirection);
        XWalkMotors motors(leftMotor, rightMotor);
        motors.arm();
        led::GpioBackend ledBackend;
        XWalkGpio ledGpio(&ledBackend, led::gpioCallbacks(), "LED");
        XWalkLed statusLed(ledGpio);
        buzzer::TestBackend buzzerBackend;
        XWalkGpio buzzerGpio(&buzzerBackend, buzzer::gpioCallbacks(), "D2");
        XWalkBuzzer buzzer(buzzerGpio);

        applyLineResponse(tracker, {1'000, 1'000, 1'000}, motors, statusLed, buzzer);
        ASSERT_TRUE(buzzer.isOn());
        ASSERT_FALSE(statusLed.isOn());
        applyLineResponse(tracker, {1'000, 200, 1'000}, motors, statusLed, buzzer);
        EXPECT_FALSE(buzzer.isOn());
        EXPECT_TRUE(statusLed.isOn());
        EXPECT_DOUBLE_EQ(motors.left().speed(), 35.0);
        EXPECT_DOUBLE_EQ(motors.right().speed(), 35.0);
    }

    /** @brief Verifies invalid paired commands do not leave one motor with stale new state. */
    TEST(XWalkSensorGroup, InvalidMotorPairCommandPreservesBothPreviousSpeeds)
    {
        using namespace xwalk::hal::test::motor;
        TestI2c bus;
        XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        XWalkPwmTimerState timerState;
        XWalkPwm leftPwm(i2c, 12U, 0x14U, timerState);
        XWalkPwm rightPwm(i2c, 13U, 0x14U, timerState);
        TestGpio leftBackend;
        TestGpio rightBackend;
        XWalkGpio leftDirection(&leftBackend, gpioCallbacks(), "D4");
        XWalkGpio rightDirection(&rightBackend, gpioCallbacks(), "D5");
        XWalkMotor leftMotor(leftPwm, leftDirection);
        XWalkMotor rightMotor(rightPwm, rightDirection);
        XWalkMotors motors(leftMotor, rightMotor);
        motors.arm();
        motors.setSpeed(12.0, 13.0);

        EXPECT_THROW(motors.setSpeed(20.0, 101.0), std::out_of_range);
        EXPECT_DOUBLE_EQ(motors.left().speed(), 12.0);
        EXPECT_DOUBLE_EQ(motors.right().speed(), 13.0);
    }

    /** @brief Verifies disarmed rejection, heartbeat refresh, timeout, and disarming without real time. */
    TEST(XWalkSensorGroup, MotorWatchdogUsesInjectedClockAndStopsDeterministically)
    {
        using namespace xwalk::hal::test::motor;
        TestI2c bus;
        XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        XWalkPwmTimerState timerState;
        XWalkPwm leftPwm(i2c, 12U, 0x14U, timerState);
        XWalkPwm rightPwm(i2c, 13U, 0x14U, timerState);
        TestGpio leftBackend;
        TestGpio rightBackend;
        XWalkGpio leftDirection(&leftBackend, gpioCallbacks(), "D4");
        XWalkGpio rightDirection(&rightBackend, gpioCallbacks(), "D5");
        XWalkMotor leftMotor(leftPwm, leftDirection);
        XWalkMotor rightMotor(rightPwm, rightDirection);
        FakeClock clock;
        XWalkMotorsConfiguration configuration;
        configuration.watchdogTimeoutMilliseconds = 100U;
        configuration.clockContext = &clock;
        configuration.clockMilliseconds = &clockMilliseconds;
        configuration.watchdogWorkerEnabled = false;
        XWalkMotors motors(leftMotor, rightMotor, configuration);

        EXPECT_FALSE(motors.isArmed());
        EXPECT_THROW(motors.forward(20.0), std::logic_error);
        motors.arm();
        motors.forward(20.0);
        clock.milliseconds = 90U;
        motors.heartbeat();
        EXPECT_THROW(motors.setSpeed(20.0, 101.0), std::out_of_range);
        clock.milliseconds = 189U;
        EXPECT_FALSE(motors.checkWatchdog());
        clock.milliseconds = 190U;
        EXPECT_TRUE(motors.checkWatchdog());
        EXPECT_FALSE(motors.isArmed());
        EXPECT_DOUBLE_EQ(leftMotor.speed(), 0.0);
        EXPECT_DOUBLE_EQ(rightMotor.speed(), 0.0);
        EXPECT_TRUE(motors.disarm());
        EXPECT_TRUE(motors.disarm());

        configuration.watchdogTimeoutMilliseconds = XHAL_RPI5CAR_MOTOR_WATCHDOG_MAXIMUM_MILLISECONDS + 1U;
        EXPECT_THROW(static_cast<void>(XWalkMotors(leftMotor, rightMotor, configuration)), std::out_of_range);
    }

    /** @brief Verifies that electrically active dual-PWM braking is armed and watchdog bounded. */
    TEST(XWalkSensorGroup, PairedBrakeRequiresArmingAndExpiresThroughWatchdog)
    {
        using namespace xwalk::hal::test::motor;
        TestI2c bus;
        XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
        XWalkPwmTimerState timerState;
        XWalkPwm leftForward(i2c, 12U, 0x14U, timerState);
        XWalkPwm leftReverse(i2c, 13U, 0x14U, timerState);
        XWalkPwm rightForward(i2c, 14U, 0x14U, timerState);
        XWalkPwm rightReverse(i2c, 15U, 0x14U, timerState);
        XWalkMotor leftMotor(leftForward, leftReverse);
        XWalkMotor rightMotor(rightForward, rightReverse);
        FakeClock clock;
        XWalkMotorsConfiguration configuration;
        configuration.watchdogTimeoutMilliseconds = 100U;
        configuration.clockContext = &clock;
        configuration.clockMilliseconds = &clockMilliseconds;
        configuration.watchdogWorkerEnabled = false;
        XWalkMotors motors(leftMotor, rightMotor, configuration);

        EXPECT_THROW(motors.brake(), std::logic_error);
        motors.arm();
        motors.brake();
        EXPECT_DOUBLE_EQ(leftForward.pulseWidthPercent(), 100.0);
        EXPECT_DOUBLE_EQ(leftReverse.pulseWidthPercent(), 100.0);
        EXPECT_DOUBLE_EQ(rightForward.pulseWidthPercent(), 100.0);
        EXPECT_DOUBLE_EQ(rightReverse.pulseWidthPercent(), 100.0);
        clock.milliseconds = 100U;
        EXPECT_TRUE(motors.checkWatchdog());
        EXPECT_DOUBLE_EQ(leftForward.pulseWidthPercent(), 0.0);
        EXPECT_DOUBLE_EQ(leftReverse.pulseWidthPercent(), 0.0);
        EXPECT_DOUBLE_EQ(rightForward.pulseWidthPercent(), 0.0);
        EXPECT_DOUBLE_EQ(rightReverse.pulseWidthPercent(), 0.0);
    }

} /* namespace */
