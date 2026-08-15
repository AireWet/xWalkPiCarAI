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

#include "xHal_Rpi5CarMath.h"
#include "xHal_Rpi5CarTrace.h"

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
    float64 XWalkServo::angleToPulseWidth(float64 angleDegrees) const noexcept
    {
        const boolean lowerHalf = angleDegrees <= configurationValue.centreAngleDegrees;
        const float64 minimumAngleDegrees =
            lowerHalf ? configurationValue.minimumAngleDegrees : configurationValue.centreAngleDegrees;
        const float64 maximumAngleDegrees =
            lowerHalf ? configurationValue.centreAngleDegrees : configurationValue.maximumAngleDegrees;
        const float64 minimumPulseWidthUs =
            lowerHalf ? configurationValue.minimumPulseWidthUs : configurationValue.centrePulseWidthUs;
        const float64 maximumPulseWidthUs =
            lowerHalf ? configurationValue.centrePulseWidthUs : configurationValue.maximumPulseWidthUs;
        const float64 inputRangeDegrees = maximumAngleDegrees - minimumAngleDegrees;
        const float64 outputRangeUs = maximumPulseWidthUs - minimumPulseWidthUs;
        const float64 angleOffsetDegrees = angleDegrees - minimumAngleDegrees;
        const float64 scaledPulseWidthUs = angleOffsetDegrees * outputRangeUs;
        const float64 pulseWidthOffsetUs = scaledPulseWidthUs / inputRangeDegrees;
        return pulseWidthOffsetUs + minimumPulseWidthUs;
    }

    void XWalkServo::validateConfiguration(const XWalkServoConfiguration& configuration)
    {
        const boolean finite =
            XHAL_IS_FINITE(configuration.minimumAngleDegrees) && XHAL_IS_FINITE(configuration.centreAngleDegrees) &&
            XHAL_IS_FINITE(configuration.maximumAngleDegrees) && XHAL_IS_FINITE(configuration.minimumPulseWidthUs) &&
            XHAL_IS_FINITE(configuration.centrePulseWidthUs) && XHAL_IS_FINITE(configuration.maximumPulseWidthUs);
        if (finite == false)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "servo.configuration values must be finite");
        }
        if (!((configuration.minimumAngleDegrees < configuration.centreAngleDegrees) &&
              (configuration.centreAngleDegrees < configuration.maximumAngleDegrees)))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "servo.angle limits must satisfy minimum < centre < maximum");
        }
        if (!((configuration.minimumPulseWidthUs < configuration.centrePulseWidthUs) &&
              (configuration.centrePulseWidthUs < configuration.maximumPulseWidthUs)))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "servo.pulse_width limits must satisfy minimum < centre < maximum");
        }
        if ((configuration.minimumPulseWidthUs < XHAL_RPI5CAR_SERVO_MIN_PULSE_US) ||
            (configuration.maximumPulseWidthUs > XHAL_RPI5CAR_SERVO_MAX_PULSE_US))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "servo.pulse_width must remain within 500 to 2500 microseconds");
        }
    }

    void XWalkServo::requireInitialized() const
    {
        if (initializedValue == false)
        {
            XWALK_HAL_ERROR(XWALK_LOGIC, "servo.initialize must succeed before an output command");
        }
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
        requireInitialized();
        const hal::boolean angleDegreesNotFinite = static_cast<hal::boolean>(!XHAL_IS_FINITE(angleDegrees));
        if (angleDegreesNotFinite)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "servo angle must be finite");
        }

        float64 calibratedAngle = angleDegrees;
        if (configurationValue.inverted)
        {
            calibratedAngle =
                configurationValue.centreAngleDegrees - (angleDegrees - configurationValue.centreAngleDegrees);
        }
        float64 clampedAngle = calibratedAngle;
        if (clampedAngle < configurationValue.minimumAngleDegrees)
        {
            clampedAngle = configurationValue.minimumAngleDegrees;
        }
        if (clampedAngle > configurationValue.maximumAngleDegrees)
        {
            clampedAngle = configurationValue.maximumAngleDegrees;
        }

        setPulseWidthTime(angleToPulseWidth(clampedAngle));
        XWALK_HAL_TRACE_UID1(RPI .184, "Servo angle configured to %.3f degrees", clampedAngle);
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
        requireInitialized();
        const hal::boolean pulseWidthUsNotFinite = static_cast<hal::boolean>(!XHAL_IS_FINITE(pulseWidthUs));
        if (pulseWidthUsNotFinite)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "servo pulse width must be finite");
        }

        float64 clampedPulseWidth = pulseWidthUs;
        if (clampedPulseWidth < configurationValue.minimumPulseWidthUs)
        {
            clampedPulseWidth = configurationValue.minimumPulseWidthUs;
        }
        if (clampedPulseWidth > configurationValue.maximumPulseWidthUs)
        {
            clampedPulseWidth = configurationValue.maximumPulseWidthUs;
        }

        const float64 servoFrameUs = static_cast<float64>(XHAL_RPI5CAR_SERVO_FRAME_US);
        const float64 servoPeriod = static_cast<float64>(XHAL_RPI5CAR_SERVO_PERIOD);
        const float64 pulseRatio = clampedPulseWidth / servoFrameUs;
        const float64 pulseWidthCount = pulseRatio * servoPeriod;
        pwmObject->setPulseWidth(pulseWidthCount);
        XWALK_HAL_TRACE_UID1(RPI .185, "Servo pulse width configured to %.3f microseconds", clampedPulseWidth);
    }

} /* namespace xwalk::hal */
