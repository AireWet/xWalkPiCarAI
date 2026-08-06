/******************************************************************************
 * @file        xHal_Rpi5CarServo.cpp
 * @brief       Implements servo angle and pulse-duration conversion.
 *
 * @details
 * Clamps finite servo commands, maps angles linearly to pulse durations, and
 * converts microseconds into truncated PWM timer counts.
 *
 * @project     xWalk Firmware
 * @module      xWalkServo
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

#include "xHal_Rpi5CarServo.h"

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
 * @brief Maps a validated servo angle to a pulse duration.
 *
 * @param[in] angleDegrees
 * Finite angle in the inclusive range -90.0 to +90.0 degrees.
 *
 * @return
 * Pulse duration in the inclusive range 500.0 to 2500.0 microseconds.
 *
 * @pre
 * `angleDegrees` has already been clamped to the supported range.
 */
float64 XWalkServo::angleToPulseWidth(float64 angleDegrees) noexcept
{
    const float64 minimumAngleDegrees = static_cast<float64>(XHAL_RPI5CAR_SERVO_MIN_ANGLE_DEG);
    const float64 maximumAngleDegrees = static_cast<float64>(XHAL_RPI5CAR_SERVO_MAX_ANGLE_DEG);
    const float64 minimumPulseWidthUs = static_cast<float64>(XHAL_RPI5CAR_SERVO_MIN_PULSE_US);
    const float64 maximumPulseWidthUs = static_cast<float64>(XHAL_RPI5CAR_SERVO_MAX_PULSE_US);
    const float64 inputRangeDegrees = maximumAngleDegrees - minimumAngleDegrees;
    const float64 outputRangeUs = maximumPulseWidthUs - minimumPulseWidthUs;
    const float64 angleOffsetDegrees = angleDegrees - minimumAngleDegrees;
    const float64 scaledPulseWidthUs = angleOffsetDegrees * outputRangeUs;
    const float64 pulseWidthOffsetUs = scaledPulseWidthUs / inputRangeDegrees;
    return pulseWidthOffsetUs + minimumPulseWidthUs;
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Sets the requested servo angle.
 *
 * @param[in] angleDegrees
 * Requested angle in degrees. Finite values are clamped to the inclusive range
 * -90.0 to +90.0 degrees.
 *
 * @post
 * The PWM channel contains the truncated count corresponding to the clamped
 * angle.
 *
 * @throws std::invalid_argument
 * If `angleDegrees` is not finite.
 */
void XWalkServo::setAngle(float64 angleDegrees)
{
    const hal::boolean angleDegreesNotFinite =
        static_cast<hal::boolean>(
            !XHAL_IS_FINITE(angleDegrees));
    if (angleDegreesNotFinite)
    {
        XHAL_THROW_INVALID_ARGUMENT("servo angle must be finite");
    }

    float64 clampedAngle = angleDegrees;
    if (clampedAngle < XHAL_RPI5CAR_SERVO_MIN_ANGLE_DEG)
    {
        clampedAngle = XHAL_RPI5CAR_SERVO_MIN_ANGLE_DEG;
    }
    if (clampedAngle > XHAL_RPI5CAR_SERVO_MAX_ANGLE_DEG)
    {
        clampedAngle = XHAL_RPI5CAR_SERVO_MAX_ANGLE_DEG;
    }

    setPulseWidthTime(angleToPulseWidth(clampedAngle));
}

/**
 * @brief Sets the servo pulse duration directly.
 *
 * @param[in] pulseWidthUs
 * Requested pulse duration in microseconds. Finite values are clamped to the
 * inclusive range 500.0 to 2500.0 microseconds.
 *
 * @post
 * The PWM channel contains the truncated count for a 20,000 microsecond frame
 * and a 4095-count period.
 *
 * @throws std::invalid_argument
 * If `pulseWidthUs` is not finite.
 */
void XWalkServo::setPulseWidthTime(float64 pulseWidthUs)
{
    const hal::boolean pulseWidthUsNotFinite =
        static_cast<hal::boolean>(
            !XHAL_IS_FINITE(pulseWidthUs));
    if (pulseWidthUsNotFinite)
    {
        XHAL_THROW_INVALID_ARGUMENT("servo pulse width must be finite");
    }

    float64 clampedPulseWidth = pulseWidthUs;
    if (clampedPulseWidth < XHAL_RPI5CAR_SERVO_MIN_PULSE_US)
    {
        clampedPulseWidth = XHAL_RPI5CAR_SERVO_MIN_PULSE_US;
    }
    if (clampedPulseWidth > XHAL_RPI5CAR_SERVO_MAX_PULSE_US)
    {
        clampedPulseWidth = XHAL_RPI5CAR_SERVO_MAX_PULSE_US;
    }

    const float64 servoFrameUs = static_cast<float64>(XHAL_RPI5CAR_SERVO_FRAME_US);
    const float64 servoPeriod = static_cast<float64>(XHAL_RPI5CAR_SERVO_PERIOD);
    const float64 pulseRatio = clampedPulseWidth / servoFrameUs;
    const float64 pulseWidthCount = pulseRatio * servoPeriod;
    pwmObject->setPulseWidth(pulseWidthCount);
}

} /* namespace xwalk::hal */
