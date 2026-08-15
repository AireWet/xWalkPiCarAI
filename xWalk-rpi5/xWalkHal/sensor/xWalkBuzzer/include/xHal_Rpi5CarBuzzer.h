/******************************************************************************
 * @file        xHal_Rpi5CarBuzzer.h
 * @brief       Declares active and passive buzzer control.
 *
 * @details
 * Provides logical on/off control for GPIO-backed active buzzers and PWM
 * frequency, duty-cycle, and timed playback control for passive buzzers.
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

#ifndef XHAL_RPI5CAR_BUZZER_H
#define XHAL_RPI5CAR_BUZZER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"
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
     * @class XWalkBuzzer
     * @brief Controls either an active GPIO buzzer or a passive PWM buzzer.
     *
     * @details
     * An active buzzer supports logical on and off operations. A passive buzzer
     * additionally supports frequency selection and optional-duration playback.
     */
    class XWalkBuzzer
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Nullable non-owning pointer to the passive buzzer PWM output.
             *
             * @details
             * Non-null only for a passive buzzer. The caller creates and owns the
             * PWM object, which must outlive this controller.
             */
            XWalkPwm* pwmObject{nullptr};

            /**
             * @brief Nullable non-owning pointer to the active buzzer GPIO output.
             *
             * @details
             * Non-null only for an active buzzer. The caller creates and owns the
             * GPIO object, which must outlive this controller.
             */
            XWalkGpio* gpioObject{nullptr};

            /** @brief Logical buzzer state after the most recent successful output operation. */
            boolean activeValue{false};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

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
             * If the duration is negative or its half-duration exceeds the
             * project unsigned 32-bit microsecond range.
             */
            static uint32 halfDurationMicroseconds(float64 durationSeconds);

            /**
             * @brief Verifies that this controller uses a passive PWM buzzer.
             *
             * @throws std::invalid_argument
             * If this controller was constructed with an active GPIO buzzer.
             */
            void requirePassiveBuzzer() const;

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a passive buzzer controller.
             *
             * @param[in] pwm
             * Non-owning PWM output used for frequency and duty-cycle control.
             *
             * @pre
             * `pwm` outlives this controller.
             *
             * @post
             * The PWM duty cycle is zero percent and the buzzer is inactive.
             */
            explicit XWalkBuzzer(XWalkPwm& pwm);

            /**
             * @brief Constructs an active buzzer controller.
             *
             * @param[in] gpio
             * Non-owning GPIO output used for logical activation.
             *
             * @pre
             * `gpio` outlives this controller.
             *
             * @post
             * The GPIO output is logically inactive.
             */
            explicit XWalkBuzzer(XWalkGpio& gpio);

            /** @brief Destroys the controller without releasing its non-owning dependency. */
            ~XWalkBuzzer();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction to preserve dependency identity. */
            XWalkBuzzer(XWalkBuzzer&&) = delete;
            /** @brief Disables copying of the non-owning dependency binding. */
            XWalkBuzzer(const XWalkBuzzer&) = delete;
            /** @brief Disables move assignment of the dependency binding. */
            XWalkBuzzer& operator=(XWalkBuzzer&&) = delete;
            /** @brief Disables copy assignment of the dependency binding. */
            XWalkBuzzer& operator=(const XWalkBuzzer&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Activates the buzzer.
             *
             * @details
             * Sets a passive buzzer to 50 percent duty cycle or drives an active
             * buzzer to its logical active GPIO level.
             *
             * @post
             * `isOn()` returns `true` after the output operation succeeds.
             */
            void on();

            /**
             * @brief Deactivates the buzzer.
             *
             * @details
             * Sets a passive buzzer to zero percent duty cycle or drives an active
             * buzzer to its logical inactive GPIO level.
             *
             * @post
             * `isOn()` returns `false` after the output operation succeeds.
             */
            void off();

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
            void setFrequency(float64 frequencyHz);

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
            void play(float64 frequencyHz);

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
            void play(float64 frequencyHz, float64 durationSeconds);

            /**
             * @brief Reports the logical buzzer state.
             *
             * @return
             * `true` after a successful activation; otherwise `false`.
             */
            boolean isOn() const noexcept;

            /**
             * @brief Reports whether the controller uses a passive PWM buzzer.
             *
             * @return
             * `true` for a PWM dependency or `false` for a GPIO dependency.
             */
            boolean isPassive() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_BUZZER_H */
