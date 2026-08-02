/******************************************************************************
 * @file        xHal_Rpi5CarServo.h
 * @brief       Declares the Robot HAT positional-servo abstraction.
 *
 * @details
 * Converts requested angles and pulse durations into PWM timer counts through
 * a caller-created `XWalkPwm` object.
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

#ifndef XHAL_RPI5CAR_SERVO_H
#define XHAL_RPI5CAR_SERVO_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarPwm.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkServo
 * @brief Converts positional-servo commands into PWM output counts.
 *
 * @details
 * Configures a supplied PWM channel for a 50 Hertz, 4095-count servo frame.
 * Angles are clamped to -90 through +90 degrees and mapped linearly to pulse
 * durations from 500 through 2500 microseconds.
 */
class XWalkServo
{
    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

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
        static float64 angleToPulseWidth(float64 angleDegrees) noexcept;

    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Non-owning pointer to the PWM channel controlled by this servo.
         *
         * @note
         * The pointer is initialized from a constructor reference, is never
         * null after construction, and must outlive this servo object.
         */
        XWalkPwm* pwmObject;

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a servo controller around a PWM channel.
         *
         * @param[in] pwm
         * Caller-created PWM channel passed by reference.
         *
         * @pre
         * `pwm` outlives this servo object.
         *
         * @post
         * The PWM timer period is 4095 counts and its prescaler is configured
         * for an approximately 50 Hertz servo frame.
         */
        explicit XWalkServo(XWalkPwm& pwm);

        /**
         * @brief Destroys the servo controller.
         *
         * @note
         * The PWM pointer is non-owning and is not released.
         */
        ~XWalkServo();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction to preserve dependency identity. */
        XWalkServo(XWalkServo&&) = delete;
        /** @brief Disables copying of the non-owning PWM binding. */
        XWalkServo(const XWalkServo&) = delete;
        /** @brief Disables move assignment of the PWM binding. */
        XWalkServo& operator=(XWalkServo&&) = delete;
        /** @brief Disables copy assignment of the PWM binding. */
        XWalkServo& operator=(const XWalkServo&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Sets the requested servo angle.
         *
         * @param[in] angleDegrees
         * Requested angle in degrees. Finite values are clamped to the inclusive
         * range -90.0 to +90.0 degrees.
         *
         * @post
         * The PWM channel contains the truncated count corresponding to the
         * clamped angle.
         *
         * @throws std::invalid_argument
         * If `angleDegrees` is not finite.
         */
        void setAngle(float64 angleDegrees);

        /**
         * @brief Sets the servo pulse duration directly.
         *
         * @param[in] pulseWidthUs
         * Requested pulse duration in microseconds. Finite values are clamped
         * to the inclusive range 500.0 to 2500.0 microseconds.
         *
         * @post
         * The PWM channel contains the truncated count for a 20,000 microsecond
         * frame and a 4095-count period.
         *
         * @throws std::invalid_argument
         * If `pulseWidthUs` is not finite.
         */
        void setPulseWidthTime(float64 pulseWidthUs);
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SERVO_H */
