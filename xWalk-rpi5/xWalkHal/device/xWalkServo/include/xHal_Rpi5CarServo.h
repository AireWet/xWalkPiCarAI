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
     * Structure declarations
     ******************************************************************************/

    /**
     * @brief Defines the calibrated angle and pulse limits for one servo.
     *
     * @details
     * Values are validated during construction, before the servo can issue a PWM
     * write. Angles outside the configured interval retain the historical servo
     * contract and are clamped to the nearest endpoint.
     */
    struct XWalkServoConfiguration
    {
            float64 minimumAngleDegrees{XHAL_RPI5CAR_SERVO_MIN_ANGLE_DEG};
            float64 centreAngleDegrees{0.0};
            float64 maximumAngleDegrees{XHAL_RPI5CAR_SERVO_MAX_ANGLE_DEG};
            float64 minimumPulseWidthUs{XHAL_RPI5CAR_SERVO_MIN_PULSE_US};
            float64 centrePulseWidthUs{1500.0};
            float64 maximumPulseWidthUs{XHAL_RPI5CAR_SERVO_MAX_PULSE_US};
            boolean inverted{false}; /**< Mirrors commanded angles around the calibrated centre. */
    };

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
             * @pre `angleDegrees` is inside `configurationValue`.
             */
            float64 angleToPulseWidth(float64 angleDegrees) const noexcept;

            /** @brief Validates calibration before any PWM operation is possible. */
            static void validateConfiguration(const XWalkServoConfiguration& configuration);

            /** @brief Rejects output commands until explicit initialization succeeds. */
            void requireInitialized() const;

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

            /** @brief Validated calibration and mechanical limits for this servo. */
            XWalkServoConfiguration configurationValue{};

            /** @brief True only after every explicit timer initialization write succeeds. */
            boolean initializedValue{false};

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
             * @param[in] configuration Validated per-servo calibration limits.
             *
             * @post No PWM command has been issued by this servo.
             */
            explicit XWalkServo(XWalkPwm& pwm, const XWalkServoConfiguration& configuration = {});

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
             * @brief Explicitly configures the PWM timer without moving the servo.
             *
             * @return `true` when this call performed initialization; `false` when
             * the servo was already initialized.
             *
             * @throws std::runtime_error Propagates a PWM or I2C initialization failure.
             */
            boolean initialize();

            /** @brief Returns whether explicit initialization completed successfully. */
            boolean isInitialized() const noexcept;

            /** @brief Explicitly moves to the configured calibrated centre angle. */
            void moveToSafePosition();

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
