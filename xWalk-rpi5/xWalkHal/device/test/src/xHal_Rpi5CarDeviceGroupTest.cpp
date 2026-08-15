/******************************************************************************
 * @file        xHal_Rpi5CarDeviceGroupTest.cpp
 * @brief       Verifies collaboration among device-group modules.
 *
 * @details
 * Composes real device abstractions over deterministic I2C, GPIO, and camera
 * fakes to verify conversion, propagation, boundary, and failure contracts.
 *
 * @project     xWalk Firmware
 * @module      xWalk Device Group Test
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

#include "xHal_Rpi5CarDeviceGroupTestSupport.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarAdcTestSupport.h"
#include "xHal_Rpi5CarAdxl345.h"
#include "xHal_Rpi5CarAdxl345TestSupport.h"
#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarCameraTestSupport.h"
#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarPwmTestI2c.h"
#include "xHal_Rpi5CarServo.h"
#include "xHal_Rpi5CarUltrasonic.h"
#include "xHal_Rpi5CarUltrasonicTestSupport.h"
#include "xHal_Rpi5CarUserButton.h"
#include "xHal_Rpi5CarUserButtonTestSupport.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "xHal_Rpi5CarDeviceGroupTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using ServoAngleCase = ::xwalk::source_types::xhal_rpi5cardevicegrouptest::ServoAngleCase;
using ServoAngleGroupTest = ::xwalk::source_types::xhal_rpi5cardevicegrouptest::ServoAngleGroupTest;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains device-group scenarios private to this translation unit. */
namespace
{

    using namespace xwalk::hal;
    using testing::ElementsAre;

    /** @brief Verifies minimum, centre, and maximum Servo commands through PWM and I2C. */
    TEST_P(ServoAngleGroupTest, MapsAngleThroughPwmToBigEndianRegisterData)
    {
        xwalk::hal::test::XWalkPwmTestI2c bus;
        XWalkI2c i2c(&bus,
                     &xwalk::hal::test::XWalkPwmTestI2c::probeCallback,
                     &xwalk::hal::test::XWalkPwmTestI2c::writeRegisterCallback,
                     &xwalk::hal::test::XWalkPwmTestI2c::readCallback);
        XWalkPwmTimerState timerState;
        XWalkPwm pwm(i2c, 0U, 0x14U, timerState);
        XWalkServo servo(pwm);
        static_cast<void>(servo.initialize());
        bus.clearWrites();

        const ServoAngleCase angleCase = GetParam();
        servo.setAngle(angleCase.angleDegrees);

        ASSERT_EQ(bus.writeCount(), 1U);
        EXPECT_EQ(bus.writeRegister(0U), 0x20U);
        EXPECT_EQ(pwm.pulseWidth(), angleCase.pulseWidth);
        const bytevector expectedBytes{static_cast<uint8>((angleCase.pulseWidth >> 8U) & 0xFFU),
                                       static_cast<uint8>(angleCase.pulseWidth & 0xFFU)};
        EXPECT_EQ(bus.writeData(0U), expectedBytes);
    }

    INSTANTIATE_TEST_SUITE_P(BoundaryAngles,
                             ServoAngleGroupTest,
                             testing::Values(ServoAngleCase{-120.0, 102U},
                                             ServoAngleCase{0.0, 307U},
                                             ServoAngleCase{120.0, 511U}));

    /** @brief Verifies an I2C write failure reaches Servo without publishing stale PWM state. */
    TEST(XWalkDeviceGroup, ServoPropagatesPwmI2cWriteFailure)
    {
        using namespace xwalk::hal::test::device_group;
        FailingI2cBackend backend;
        XWalkI2c i2c(&backend, &probe, &writeRegister, &read);
        XWalkPwmTimerState timerState;
        XWalkPwm pwm(i2c, 0U, 0x14U, timerState);
        XWalkServo servo(pwm);
        backend.failingWrite = backend.writeCount + 1U;

        EXPECT_THROW(servo.initialize(), std::runtime_error);
        EXPECT_FALSE(servo.isInitialized());
        backend.failingWrite = 0U;
        EXPECT_TRUE(servo.initialize());
        backend.failingWrite = backend.writeCount + 1U;

        EXPECT_THROW(servo.setAngle(45.0), std::runtime_error);
        EXPECT_EQ(backend.writeCount, backend.failingWrite);
        backend.failingWrite = 0U;
        servo.setAngle(0.0);
        EXPECT_EQ(pwm.pulseWidth(), 307U) << "Servo must remain usable after the propagated bus failure is cleared";
    }

    /** @brief Verifies ADC byte assembly and ADXL345 signed little-endian scaling. */
    TEST(XWalkDeviceGroup, AdcAndAccelerometerPreserveBusEncoding)
    {
        xwalk::hal::test::adc::TestBus adcBus;
        XWalkI2c adcI2c(&adcBus,
                        &xwalk::hal::test::adc::probe,
                        &xwalk::hal::test::adc::writeRegister,
                        &xwalk::hal::test::adc::read);
        XWalkAdc adc(adcI2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        EXPECT_EQ(adc.read(), 0x0ABCU);
        EXPECT_EQ(adcBus.writeRegister, 0x17U);
        EXPECT_THAT(adcBus.writeData, ElementsAre(0x00U, 0x00U));

        xwalk::hal::test::adxl345::TestBus accelerometerBus;
        accelerometerBus.responses = {{0x00U, 0x00U}, {0x00U, 0xFFU}};
        XWalkI2c accelerometerI2c(&accelerometerBus,
                                  &xwalk::hal::test::adxl345::probe,
                                  &xwalk::hal::test::adxl345::writeRegister,
                                  &xwalk::hal::test::adxl345::read,
                                  &xwalk::hal::test::adxl345::readRegister);
        XWalkAdxl345 accelerometer(accelerometerI2c);
        EXPECT_DOUBLE_EQ(accelerometer.read(XWalkAdxl345Axis::Y), -1.0);
        EXPECT_EQ(accelerometerBus.lastRegister, XHAL_RPI5CAR_ADXL345_DATA_Y_REGISTER);
        EXPECT_EQ(accelerometerBus.registerReadCount, 2U);

        adcBus.readBytes = {0x01U};
        EXPECT_THROW(static_cast<void>(adc.read()), std::runtime_error);
        accelerometerBus.responses = {{0x00U}};
        accelerometerBus.responseIndex = 0U;
        EXPECT_THROW(static_cast<void>(accelerometer.read(XWalkAdxl345Axis::X)), std::runtime_error);
    }

    /** @brief Verifies GPIO trigger/echo success, timeout, and invalid-pulse results. */
    TEST(XWalkDeviceGroup, UltrasonicConvertsOnlyCompleteEchoPulses)
    {
        using namespace xwalk::hal::test::ultrasonic;
        TestBackend backend;
        backend.pulseDelayMicroseconds = 100U;
        const XWalkGpioCallbacks callbackSet = callbacks();
        XWalkGpio trigger(&backend, callbackSet, "D2");
        XWalkGpio echo(&backend, callbackSet, "D3");
        XWalkUltrasonic ultrasonic(trigger, echo, 20'000U);

        const float64 distanceCentimeters = ultrasonic.read(1U);
        const float64 maximumTimeoutDistanceCentimeters = 350.0;
        EXPECT_GT(distanceCentimeters, 1.0);
        EXPECT_LT(distanceCentimeters, maximumTimeoutDistanceCentimeters);
        EXPECT_THAT(backend.triggerLevels, ElementsAre(0U, 1U, 0U));

        backend.behavior = EchoBehavior::Timeout;
        backend.triggerCount = 0U;
        EXPECT_EQ(ultrasonic.read(2U), XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM);
        EXPECT_EQ(backend.triggerCount, 2U);

        backend.behavior = EchoBehavior::Invalid;
        backend.triggerCount = 0U;
        EXPECT_EQ(ultrasonic.read(3U), XHAL_RPI5CAR_ULTRASONIC_INVALID_PULSE_RESULT_CM);
        EXPECT_EQ(backend.triggerCount, 1U);
    }

    /** @brief Verifies camera capture forwarding and backend failure propagation. */
    TEST(XWalkDeviceGroup, CameraUsesOnlyInjectedCaptureBoundary)
    {
        using namespace xwalk::hal::test::camera;
        CameraTestState backend;
        XWalkCamera camera(&backend, &captureImage);
        EXPECT_EQ(camera.capture("group-frame.jpg"), "group-frame.jpg");
        EXPECT_EQ(backend.captureCount, 1U);
        EXPECT_EQ(backend.configuration.widthPixels, 640U);
        EXPECT_EQ(backend.configuration.heightPixels, 480U);

        backend.result = false;
        EXPECT_THROW(static_cast<void>(camera.capture("failed-frame.jpg")), std::runtime_error);
        EXPECT_EQ(backend.captureCount, 2U);
    }

    /** @brief Verifies UserButton GPIO role and threshold boundary behavior without polling. */
    TEST(XWalkDeviceGroup, UserButtonPreservesActiveLowGpioAndThresholdBoundaries)
    {
        using namespace xwalk::hal::test::userbutton;
        TestBackend backend;
        XWalkGpio gpio(&backend, gpioCallbacks(), "USER", XWalkGpioMode::Input, XWalkGpioPull::Up);
        XWalkUserButton button(gpio);
        EXPECT_EQ(gpio.mode(), XWalkGpioMode::Input);
        EXPECT_EQ(gpio.pull(), XWalkGpioPull::Up);
        EXPECT_FALSE(button.isPressed());

        button.setOnLongPress(nullptr, nullptr, 1.0);
        EXPECT_DOUBLE_EQ(button.longPressDurationSeconds(), 2.0);
        button.setOnLongPress(nullptr, nullptr, 8.0);
        EXPECT_DOUBLE_EQ(button.longPressDurationSeconds(), 5.0);
        EXPECT_THROW(button.setOnLongPress(nullptr, nullptr, XHAL_POSITIVE_INFINITY(float64)), std::invalid_argument);
        EXPECT_FALSE(button.isRunning());
    }

} /* namespace */
