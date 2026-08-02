/******************************************************************************
 * @file        xHal_Rpi5CarMotor.cpp
 * @brief       Implements single-motor speed, direction, reversal, and braking.
 *
 * @details
 * Converts signed speed commands to PWM duty cycles and GPIO or secondary-PWM direction outputs.
 *
 * @project     xWalk Firmware
 * @module      xWalkMotor
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

#include "xHal_Rpi5CarMotor.h"
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarMath.h"

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
 * @brief Applies a signed motor speed command.
 *
 * @param[in] speedPercent
 * Signed speed in the inclusive range -100.0 to 100.0 percent.
 *
 * @post
 * Direction outputs and PWM duty cycles represent the validated command, and `speed()` returns it.
 */
void XWalkMotor::setSpeed(float64 speedPercent)
{
    const float64 validatedSpeedPercent = validateSpeed(speedPercent);
    const float64 dutyCyclePercent = XHAL_ABSOLUTE_VALUE(validatedSpeedPercent);
    boolean forwardDirection = validatedSpeedPercent > 0.0;
    if (reversedValue)
    {
        forwardDirection = !forwardDirection;
    }

    if (modeValue == XWalkMotorMode::PwmAndDirection)
    {
        pwmA->setPulseWidthPercent(dutyCyclePercent);
        static_cast<void>(directionPin->write(forwardDirection));
    }
    else if (forwardDirection)
    {
        pwmA->setPulseWidthPercent(dutyCyclePercent);
        pwmB->setPulseWidthPercent(0.0);
    }
    else
    {
        pwmA->setPulseWidthPercent(0.0);
        pwmB->setPulseWidthPercent(dutyCyclePercent);
    }
    speedPercentValue = validatedSpeedPercent;
}

/**
 * @brief Commands a zero-percent duty cycle without changing reversal configuration.
 *
 * @post
 * `speed()` returns zero and every active speed PWM output has zero-percent duty cycle.
 */
void XWalkMotor::stop()
{
    if (!stopSafely())
    {
        XHAL_THROW_RUNTIME_ERROR("motor stop could not disable every PWM output");
    }
}

/**
 * @brief Makes a non-throwing best-effort attempt to disable every speed PWM output.
 *
 * @return
 * `true` when every required PWM output accepted zero percent; otherwise `false`.
 *
 * @post
 * Every required PWM output has received an independent zero-percent attempt. `speed()` returns zero only
 * when all required outputs accepted the request.
 *
 * @note
 * Exception suppression is restricted to this fail-safe boundary so destruction and emergency cleanup can
 * continue attempting every independent output.
 */
boolean XWalkMotor::stopSafely() noexcept
{
    const boolean primaryStopped = pwmA->trySetPulseWidthPercent(0.0);
    boolean secondaryStopped{modeValue == XWalkMotorMode::PwmAndDirection};
    if (pwmB != nullptr)
    {
        secondaryStopped = pwmB->trySetPulseWidthPercent(0.0);
    }
    const boolean stopped = primaryStopped && secondaryStopped;
    if (stopped)
    {
        speedPercentValue = 0.0;
    }
    return stopped;
}

/**
 * @brief Electrically brakes a dual-PWM motor.
 *
 * @post
 * Both dual-PWM inputs have 100-percent duty cycle and `speed()` returns zero.
 *
 * @throws std::invalid_argument
 * If this object uses PWM-and-direction mode.
 */
void XWalkMotor::brake()
{
    if (modeValue != XWalkMotorMode::DualPwm)
    {
        XHAL_THROW_INVALID_ARGUMENT("motor braking requires dual-PWM mode");
    }
    pwmA->setPulseWidthPercent(100.0);
    pwmB->setPulseWidthPercent(100.0);
    speedPercentValue = 0.0;
}

/**
 * @brief Configures logical direction reversal.
 *
 * @param[in] reversed
 * `true` to exchange logical forward and reverse.
 *
 * @post
 * Future speed commands use the requested direction mapping.
 */
void XWalkMotor::setReversed(boolean reversed) noexcept
{
    reversedValue = reversed;
}

/**
 * @brief Returns the last successfully applied signed speed.
 *
 * @return
 * Speed in the inclusive range -100.0 to 100.0 percent.
 */
float64 XWalkMotor::speed() const noexcept
{
    return speedPercentValue;
}

/**
 * @brief Returns whether logical direction is reversed.
 *
 * @return
 * `true` when forward and reverse are exchanged; otherwise `false`.
 */
boolean XWalkMotor::reversed() const noexcept
{
    return reversedValue;
}

/**
 * @brief Returns the motor-driver mode.
 *
 * @return
 * Mode selected by the constructor overload.
 */
XWalkMotorMode XWalkMotor::mode() const noexcept
{
    return modeValue;
}

/**
 * @brief Returns the configured PWM frequency.
 *
 * @return
 * Requested motor frequency in Hertz.
 */
float64 XWalkMotor::frequency() const noexcept
{
    return frequencyHzValue;
}

} /* namespace xwalk::hal */
