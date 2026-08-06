/******************************************************************************
 * @file        xHal_Rpi5CarMotorLifecycle.cpp
 * @brief       Implements single-motor validation and lifecycle behavior.
 *
 * @details
 * Validates speed and frequency inputs, binds caller-owned hardware, and
 * initializes the selected motor-driver outputs.
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
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates a signed motor speed command.
 *
 * @param[in] speedPercent
 * Signed speed in the inclusive range -100.0 to 100.0 percent.
 *
 * @return
 * Validated speed command.
 *
 * @throws std::invalid_argument
 * If the value is not finite.
 *
 * @throws std::out_of_range
 * If the value is outside the permitted range.
 */
float64 XWalkMotor::validateSpeed(float64 speedPercent)
{
    const hal::boolean speedPercentNotFinite =
        static_cast<hal::boolean>(
            !XHAL_IS_FINITE(speedPercent));
    if (speedPercentNotFinite)
    {
        XHAL_THROW_INVALID_ARGUMENT("motor speed must be finite");
    }
    if ((speedPercent < XHAL_RPI5CAR_MOTOR_MIN_SPEED_PERCENT) ||
        (speedPercent > XHAL_RPI5CAR_MOTOR_MAX_SPEED_PERCENT))
    {
        XHAL_THROW_OUT_OF_RANGE("motor speed must be between -100 and 100 percent");
    }
    return speedPercent;
}

/**
 * @brief Validates a motor PWM frequency.
 *
 * @param[in] frequencyHz
 * Finite frequency greater than zero, in Hertz.
 *
 * @return
 * Validated frequency in Hertz.
 *
 * @throws std::invalid_argument
 * If the frequency is non-finite or not greater than zero.
 */
float64 XWalkMotor::validateFrequency(float64 frequencyHz)
{
    const hal::boolean frequencyHzInvalid =
        static_cast<hal::boolean>(
            (!XHAL_IS_FINITE(frequencyHz)) || (frequencyHz <= 0.0));
    if (frequencyHzInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("motor frequency must be finite and greater than zero");
    }
    return frequencyHz;
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a PWM-and-direction motor driver.
 *
 * @param[in] pwm
 * Primary PWM dependency that must outlive this motor.
 *
 * @param[in] direction
 * Digital direction dependency that must outlive this motor.
 *
 * @param[in] reversed
 * `true` to exchange the logical forward and reverse directions.
 *
 * @param[in] frequencyHz
 * Finite PWM frequency greater than zero, in Hertz.
 *
 * @post
 * The PWM channel uses `frequencyHz` and zero-percent duty cycle.
 */
XWalkMotor::XWalkMotor(XWalkPwm& pwm, XWalkGpio& direction, boolean reversed, float64 frequencyHz):
    pwmA(&pwm), directionPin(&direction), modeValue(XWalkMotorMode::PwmAndDirection),
    frequencyHzValue(validateFrequency(frequencyHz)), reversedValue(reversed)
{
    pwmA->setFrequency(frequencyHzValue);
    pwmA->setPulseWidthPercent(0.0);
}

/**
 * @brief Constructs a dual-PWM motor driver.
 *
 * @param[in] forwardPwm
 * Forward PWM dependency that must outlive this motor.
 *
 * @param[in] reversePwm
 * Reverse PWM dependency that must outlive this motor.
 *
 * @param[in] reversed
 * `true` to exchange the logical forward and reverse directions.
 *
 * @param[in] frequencyHz
 * Finite PWM frequency greater than zero, in Hertz.
 *
 * @post
 * Both PWM channels use `frequencyHz` and zero-percent duty cycle.
 */
XWalkMotor::XWalkMotor(XWalkPwm& forwardPwm, XWalkPwm& reversePwm, boolean reversed,
    float64 frequencyHz):
    pwmA(&forwardPwm), pwmB(&reversePwm), modeValue(XWalkMotorMode::DualPwm),
    frequencyHzValue(validateFrequency(frequencyHz)), reversedValue(reversed)
{
    pwmA->setFrequency(frequencyHzValue);
    pwmA->setPulseWidthPercent(0.0);
    pwmB->setFrequency(frequencyHzValue);
    pwmB->setPulseWidthPercent(0.0);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Makes one non-throwing stop attempt and releases no non-owning dependency.
 */
XWalkMotor::~XWalkMotor() noexcept
{
    static_cast<void>(stopSafely());
}


} /* namespace xwalk::hal */
