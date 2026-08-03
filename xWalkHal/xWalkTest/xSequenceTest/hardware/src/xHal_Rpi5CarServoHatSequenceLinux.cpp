/******************************************************************************
 * @file        xHal_Rpi5CarServoHatSequenceLinux.cpp
 * @brief       Implements Linux composition for the Robot HAT servo sequence.
 *
 * @details
 * Creates the reset and speaker GPIO roles, one Robot HAT I2C interface,
 * sixteen PWM servos, and five ADC inputs before running the bounded sequence.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarServoHatSequenceLinux.h"

#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarI2cLinux.h"

#include <iostream>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains callbacks private to the Linux composition unit. */
namespace
{

/**
 * @brief Provides the unused speaker-prime operation required by board control.
 *
 * @param[in,out] context
 * Unused callback context.
 *
 * @param[in] durationMilliseconds
 * Unused requested prime duration in milliseconds.
 */
void unusedSpeakerPrime(
    XWalkHal::contextpointer context, XWalkHal::uint32 durationMilliseconds)
{
    static_cast<void>(context);
    static_cast<void>(durationMilliseconds);
}

} /* namespace */

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

/**
 * @brief Runs the bounded physical servo sweep and ADC monitor.
 *
 * @param[in] i2cDevice
 * Linux I2C character-device path.
 *
 * @param[in] gpioDevice
 * Linux GPIO chip-device path.
 *
 * @param[in] gpioChipName
 * Optional exact GPIO chip name.
 *
 * @param[in] gpioChipLabel
 * Optional exact GPIO chip label.
 *
 * @param[in] sampleCount
 * ADC sample count in the inclusive range 1 through 3600.
 *
 * @warning
 * This operation resets the MCU and physically moves all 16 servo channels.
 */
void XWalkServoHatSequenceLinux::run(cstring i2cDevice, cstring gpioDevice,
    cstring gpioChipName, cstring gpioChipLabel, uint32 sampleCount)
{
    XWalkI2cLinux i2cBackend(i2cDevice);
    XWalkI2c i2c(&i2cBackend, XHAL_I2C_PROBE_CALLBACK(XWalkI2cLinux),
        XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkI2cLinux),
        XHAL_I2C_READ_CALLBACK(XWalkI2cLinux));

    const XWalkGpioCallbacks gpioCallbacks = XHAL_GPIO_CALLBACKS(XWalkGpioLinux);
    XWalkGpioLinux resetBackend(gpioDevice, gpioChipName, gpioChipLabel, 28U);
    XWalkGpio resetGpio(&resetBackend, gpioCallbacks, "MCURST");
    XWalkGpioLinux speakerBackend(gpioDevice, gpioChipName, gpioChipLabel, 28U);
    XWalkGpio speakerGpio(&speakerBackend, gpioCallbacks,
        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);

    XWalkAdc adc0(i2c, 0U);
    XWalkAdc adc1(i2c, 1U);
    XWalkAdc adc2(i2c, 2U);
    XWalkAdc adc3(i2c, 3U);
    XWalkAdc adc4(i2c, 4U);
    XWalkBoardControl boardControl(resetGpio, speakerGpio, adc4,
        nullptr, &unusedSpeakerPrime);

    XWalkPwmTimerState timerState;
    XWalkPwm pwm0(i2c, 0U, {}, timerState);
    XWalkPwm pwm1(i2c, 1U, {}, timerState);
    XWalkPwm pwm2(i2c, 2U, {}, timerState);
    XWalkPwm pwm3(i2c, 3U, {}, timerState);
    XWalkPwm pwm4(i2c, 4U, {}, timerState);
    XWalkPwm pwm5(i2c, 5U, {}, timerState);
    XWalkPwm pwm6(i2c, 6U, {}, timerState);
    XWalkPwm pwm7(i2c, 7U, {}, timerState);
    XWalkPwm pwm8(i2c, 8U, {}, timerState);
    XWalkPwm pwm9(i2c, 9U, {}, timerState);
    XWalkPwm pwm10(i2c, 10U, {}, timerState);
    XWalkPwm pwm11(i2c, 11U, {}, timerState);
    XWalkPwm pwm12(i2c, 12U, {}, timerState);
    XWalkPwm pwm13(i2c, 13U, {}, timerState);
    XWalkPwm pwm14(i2c, 14U, {}, timerState);
    XWalkPwm pwm15(i2c, 15U, {}, timerState);

    XWalkServo servo0(pwm0);
    XWalkServo servo1(pwm1);
    XWalkServo servo2(pwm2);
    XWalkServo servo3(pwm3);
    XWalkServo servo4(pwm4);
    XWalkServo servo5(pwm5);
    XWalkServo servo6(pwm6);
    XWalkServo servo7(pwm7);
    XWalkServo servo8(pwm8);
    XWalkServo servo9(pwm9);
    XWalkServo servo10(pwm10);
    XWalkServo servo11(pwm11);
    XWalkServo servo12(pwm12);
    XWalkServo servo13(pwm13);
    XWalkServo servo14(pwm14);
    XWalkServo servo15(pwm15);

    const servohatservoarray servos{{&servo0, &servo1, &servo2, &servo3,
        &servo4, &servo5, &servo6, &servo7, &servo8, &servo9, &servo10,
        &servo11, &servo12, &servo13, &servo14, &servo15}};
    const servohatadcarray adcInputs{{&adc0, &adc1, &adc2, &adc3, &adc4}};
    XWalkServoHatSequence sequence(boardControl, servos, adcInputs, this,
        &XWalkServoHatSequenceLinux::wait,
        &XWalkServoHatSequenceLinux::reportServo,
        &XWalkServoHatSequenceLinux::reportAdc);
    sequence.run(sampleCount);
}

/**
 * @brief Waits for the requested duration.
 *
 * @param[in,out] context
 * Unused callback context.
 *
 * @param[in] durationMilliseconds
 * Requested wait duration in milliseconds.
 */
void XWalkServoHatSequenceLinux::wait(
    contextpointer context, uint32 durationMilliseconds)
{
    static_cast<void>(context);
    common::sleepMilliseconds(durationMilliseconds);
}

/**
 * @brief Prints the upstream servo-channel status message.
 *
 * @param[in,out] context
 * Unused callback context.
 *
 * @param[in] channel
 * Servo PWM channel in the range zero through 15.
 */
void XWalkServoHatSequenceLinux::reportServo(
    contextpointer context, uint8 channel)
{
    static_cast<void>(context);
    std::cout << "Servo " << static_cast<uint32>(channel) << " set to zero\n";
}

/**
 * @brief Prints one ordered five-channel ADC sample.
 *
 * @param[in,out] context
 * Unused callback context.
 *
 * @param[in] readings
 * Raw ADC readings ordered by channels zero through four.
 */
void XWalkServoHatSequenceLinux::reportAdc(
    contextpointer context, const servohatreadings& readings)
{
    static_cast<void>(context);
    std::cout << readings[0U] << ' ' << readings[1U] << ' '
              << readings[2U] << ' ' << readings[3U] << ' '
              << readings[4U] << '\n';
}

} /* namespace xwalk::hal::test */
