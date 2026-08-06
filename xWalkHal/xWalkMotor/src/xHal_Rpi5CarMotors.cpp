/******************************************************************************
 * @file        xHal_Rpi5CarMotors.cpp
 * @brief       Implements coordinated left and right motor control.
 *
 * @details
 * Applies paired speed, movement, braking, role assignment, and reversal operations to caller-owned motors.
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

#include "xHal_Rpi5CarMotors.h"
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
 * @brief Stops both configured drive motors.
 *
 * @post
 * Both motors contain a zero-percent signed speed command.
 */
void XWalkMotors::stop()
{
    const hal::boolean stopped = stopSafely();
    if (stopped == false)
    {
        XHAL_THROW_RUNTIME_ERROR("paired motor stop could not disable every PWM output");
    }
}

/**
 * @brief Makes independent non-throwing stop attempts for both drive motors.
 *
 * @return
 * `true` when both motors accepted every required zero-output request; otherwise `false`.
 *
 * @post
 * Both configured motors have received a stop attempt even if either attempt fails.
 */
boolean XWalkMotors::stopSafely() noexcept
{
    const boolean firstStopped = motorOne->stopSafely();
    const boolean secondStopped = motorTwo->stopSafely();
    return firstStopped && secondStopped;
}

/**
 * @brief Brakes both configured drive motors.
 *
 * @post
 * Both dual-PWM motors have their two inputs driven to 100-percent duty cycle.
 *
 * @throws std::invalid_argument
 * If either motor is not configured for dual-PWM mode.
 */
void XWalkMotors::brake()
{
    const hal::boolean motorOneModeXWalkMotorModeInvalid =
        static_cast<hal::boolean>(
            (motorOne->mode() != XWalkMotorMode::DualPwm) || (motorTwo->mode() != XWalkMotorMode::DualPwm));
    if (motorOneModeXWalkMotorModeInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("paired braking requires two dual-PWM motors");
    }
    motorOne->brake();
    motorTwo->brake();
}

/**
 * @brief Applies independent signed left and right speed commands.
 *
 * @param[in] leftSpeedPercent
 * Left speed in the inclusive range -100.0 to 100.0 percent.
 *
 * @param[in] rightSpeedPercent
 * Right speed in the inclusive range -100.0 to 100.0 percent.
 *
 * @post
 * The assigned left and right motors contain their respective validated commands.
 */
void XWalkMotors::setSpeed(float64 leftSpeedPercent, float64 rightSpeedPercent)
{
    validateSpeeds(leftSpeedPercent, rightSpeedPercent);
    left().setReversed(configurationValue.leftReversed);
    right().setReversed(configurationValue.rightReversed);
    left().setSpeed(leftSpeedPercent);
    right().setSpeed(rightSpeedPercent);
}

/**
 * @brief Drives both motors forward at the supplied magnitude.
 *
 * @param[in] speedPercent
 * Signed value in the range -100.0 to 100.0 percent; negative input preserves Python sign behavior.
 */
void XWalkMotors::forward(float64 speedPercent)
{
    setSpeed(speedPercent, speedPercent);
}

/**
 * @brief Drives both motors backward at the supplied magnitude.
 *
 * @param[in] speedPercent
 * Signed value in the range -100.0 to 100.0 percent; the command negates this value.
 */
void XWalkMotors::backward(float64 speedPercent)
{
    setSpeed(-speedPercent, -speedPercent);
}

/**
 * @brief Commands a counter-rotating left turn.
 *
 * @param[in] speedPercent
 * Signed value in the range -100.0 to 100.0 percent.
 */
void XWalkMotors::turnLeft(float64 speedPercent)
{
    setSpeed(-speedPercent, speedPercent);
}

/**
 * @brief Commands a counter-rotating right turn.
 *
 * @param[in] speedPercent
 * Signed value in the range -100.0 to 100.0 percent.
 */
void XWalkMotors::turnRight(float64 speedPercent)
{
    setSpeed(speedPercent, -speedPercent);
}

/**
 * @brief Assigns motor 1 or 2 to the left side.
 *
 * @param[in] motorId
 * One-based motor identifier, either 1 or 2.
 */
void XWalkMotors::setLeftMotorId(uint8 motorId)
{
    configurationValue.leftMotorId = validateMotorId(motorId);
    left().setReversed(configurationValue.leftReversed);
}

/**
 * @brief Assigns motor 1 or 2 to the right side.
 *
 * @param[in] motorId
 * One-based motor identifier, either 1 or 2.
 */
void XWalkMotors::setRightMotorId(uint8 motorId)
{
    configurationValue.rightMotorId = validateMotorId(motorId);
    right().setReversed(configurationValue.rightReversed);
}

/**
 * @brief Sets and applies the left motor reversal state.
 *
 * @param[in] reversed
 * Requested left motor reversal state.
 */
void XWalkMotors::setLeftReversed(boolean reversed) noexcept
{
    configurationValue.leftReversed = reversed;
    left().setReversed(reversed);
}

/**
 * @brief Sets and applies the right motor reversal state.
 *
 * @param[in] reversed
 * Requested right motor reversal state.
 */
void XWalkMotors::setRightReversed(boolean reversed) noexcept
{
    configurationValue.rightReversed = reversed;
    right().setReversed(reversed);
}

/**
 * @brief Toggles and returns the left motor reversal state.
 *
 * @return
 * Updated left motor reversal state.
 */
boolean XWalkMotors::toggleLeftReversed() noexcept
{
    setLeftReversed(!configurationValue.leftReversed);
    return configurationValue.leftReversed;
}

/**
 * @brief Toggles and returns the right motor reversal state.
 *
 * @return
 * Updated right motor reversal state.
 */
boolean XWalkMotors::toggleRightReversed() noexcept
{
    setRightReversed(!configurationValue.rightReversed);
    return configurationValue.rightReversed;
}

/**
 * @brief Returns the motor assigned to the left side.
 *
 * @return
 * Non-owning reference to the configured left motor.
 */
XWalkMotor& XWalkMotors::left()
{
    return motor(configurationValue.leftMotorId);
}

/**
 * @brief Returns the motor assigned to the right side.
 *
 * @return
 * Non-owning reference to the configured right motor.
 */
XWalkMotor& XWalkMotors::right()
{
    return motor(configurationValue.rightMotorId);
}

/**
 * @brief Returns motor 1 or 2 by one-based identifier.
 *
 * @param[in] motorId
 * One-based motor identifier, either 1 or 2.
 *
 * @return
 * Non-owning reference to the selected motor.
 */
XWalkMotor& XWalkMotors::motor(uint8 motorId)
{
    const uint8 validatedMotorId = validateMotorId(motorId);
    return (validatedMotorId == XHAL_RPI5CAR_MOTOR_FIRST_ID) ? *motorOne : *motorTwo;
}

/**
 * @brief Returns a copy of the runtime configuration for external persistence.
 *
 * @return
 * Current role identifiers and reversal states.
 */
XWalkMotorsConfiguration XWalkMotors::configuration() const noexcept
{
    return configurationValue;
}

} /* namespace xwalk::hal */
