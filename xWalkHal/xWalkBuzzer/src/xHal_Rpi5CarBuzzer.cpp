/******************************************************************************
 * @file        xHal_Rpi5CarBuzzer.cpp
 * @brief       Implements active and passive buzzer operations.
 *
 * @details
 * Routes logical activation through GPIO or PWM, configures passive-buzzer
 * frequency, and provides continuous or optional-duration playback.
 *
 * @project     xWalk Firmware
 * @module      xWalkBuzzer
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

#include "xHal_Rpi5CarBuzzer.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Activates the buzzer.
 *
 * @details
 * Sets a passive buzzer to 50 percent duty cycle or drives an active buzzer to
 * its logical active GPIO level.
 *
 * @post
 * `isOn()` returns `true` after the output operation succeeds.
 */
void XWalkBuzzer::on()
{
    if (pwmObject != nullptr)
    {
        pwmObject->setPulseWidthPercent(XHAL_RPI5CAR_BUZZER_ON_DUTY_CYCLE_PERCENT);
    }
    else
    {
        static_cast<void>(gpioObject->on());
    }
    activeValue = true;
}

/**
 * @brief Deactivates the buzzer.
 *
 * @details
 * Sets a passive buzzer to zero percent duty cycle or drives an active buzzer
 * to its logical inactive GPIO level.
 *
 * @post
 * `isOn()` returns `false` after the output operation succeeds.
 */
void XWalkBuzzer::off()
{
    if (pwmObject != nullptr)
    {
        pwmObject->setPulseWidthPercent(XHAL_RPI5CAR_BUZZER_OFF_DUTY_CYCLE_PERCENT);
    }
    else
    {
        static_cast<void>(gpioObject->off());
    }
    activeValue = false;
}

/**
 * @brief Configures the passive buzzer frequency.
 *
 * @param[in] frequencyHz
 * Finite frequency greater than zero, in Hertz.
 *
 * @throws std::invalid_argument
 * If the buzzer is active rather than passive, or the PWM frequency is
 * non-finite or not greater than zero.
 *
 * @throws std::out_of_range
 * If no valid PWM timer configuration represents the frequency.
 */
void XWalkBuzzer::setFrequency(float64 frequencyHz)
{
    requirePassiveBuzzer();
    pwmObject->setFrequency(frequencyHz);
}

/**
 * @brief Plays a passive buzzer continuously at a requested frequency.
 *
 * @param[in] frequencyHz
 * Finite frequency greater than zero, in Hertz.
 *
 * @post
 * The passive buzzer remains active until `off()` is called.
 *
 * @throws std::invalid_argument
 * If this controller uses an active buzzer or the frequency is invalid.
 */
void XWalkBuzzer::play(float64 frequencyHz)
{
    setFrequency(frequencyHz);
    on();
}

/**
 * @brief Plays one passive-buzzer cycle with equal sounding and silent halves.
 *
 * @param[in] frequencyHz
 * Finite frequency greater than zero, in Hertz.
 *
 * @param[in] durationSeconds
 * Finite total cycle duration in seconds, greater than or equal to zero.
 *
 * @post
 * The buzzer is inactive after both half-duration intervals complete.
 *
 * @throws std::invalid_argument
 * If this controller uses an active buzzer, or an argument is non-finite.
 *
 * @throws std::out_of_range
 * If the duration is negative, its converted half-duration exceeds the
 * supported range, or the frequency cannot be represented.
 */
void XWalkBuzzer::play(float64 frequencyHz, float64 durationSeconds)
{
    requirePassiveBuzzer();
    const uint32 halfDurationUs = halfDurationMicroseconds(durationSeconds);
    setFrequency(frequencyHz);
    on();
    common::sleepMicroseconds(halfDurationUs);
    off();
    common::sleepMicroseconds(halfDurationUs);
}

/**
 * @brief Reports the logical buzzer state.
 *
 * @return
 * `true` after a successful activation; otherwise `false`.
 */
boolean XWalkBuzzer::isOn() const noexcept
{
    return activeValue;
}

/**
 * @brief Reports whether the controller uses a passive PWM buzzer.
 *
 * @return
 * `true` for a PWM dependency or `false` for a GPIO dependency.
 */
boolean XWalkBuzzer::isPassive() const noexcept
{
    return pwmObject != nullptr;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Converts a total playback duration to one half-delay.
 *
 * @param[in] durationSeconds
 * Finite total duration in seconds, greater than or equal to zero.
 *
 * @return
 * Rounded half-duration in microseconds.
 *
 * @throws std::invalid_argument
 * If the duration is not finite.
 *
 * @throws std::out_of_range
 * If the duration is negative or its half-duration exceeds the project
 * unsigned 32-bit microsecond range.
 */
uint32 XWalkBuzzer::halfDurationMicroseconds(float64 durationSeconds)
{
    const hal::boolean durationSecondsNotFinite =
        static_cast<hal::boolean>(
            !XHAL_IS_FINITE(durationSeconds));
    if (durationSecondsNotFinite)
    {
        XHAL_THROW_INVALID_ARGUMENT("Buzzer duration must be finite");
    }
    if (durationSeconds < 0.0)
    {
        XHAL_THROW_OUT_OF_RANGE("Buzzer duration must not be negative");
    }

    const float64 durationMicroseconds =
        durationSeconds * XHAL_RPI5CAR_BUZZER_MICROSECONDS_PER_SECOND;
    const float64 halfDurationUs =
        durationMicroseconds / XHAL_RPI5CAR_BUZZER_DURATION_HALF_DIVISOR;
    return common::roundedValue(halfDurationUs, "Buzzer half-duration", 0U,
        XHAL_RPI5CAR_UINT32_MAX);
}

/**
 * @brief Verifies that this controller uses a passive PWM buzzer.
 *
 * @throws std::invalid_argument
 * If this controller was constructed with an active GPIO buzzer.
 */
void XWalkBuzzer::requirePassiveBuzzer() const
{
    if (pwmObject == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Buzzer operation requires a passive PWM buzzer");
    }
}

} /* namespace xwalk::hal */
