/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonic.h
 * @brief       Declares the xWalk ultrasonic distance sensor interface.
 *
 * @details
 * Defines trigger-pulse generation, echo timing, timeout handling, and
 * distance conversion for a two-pin ultrasonic ranging sensor.
 *
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic
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

#ifndef XHAL_RPI5CAR_ULTRASONIC_H
#define XHAL_RPI5CAR_ULTRASONIC_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarGpio.h"

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
     * @class XWalkUltrasonic
     * @brief Measures distance using caller-owned trigger and echo GPIO objects.
     *
     * @details
     * Generates a bounded trigger pulse, polls the echo input using a monotonic
     * clock, and converts the round-trip pulse duration to centimeters. The class
     * stores non-owning GPIO pointers and performs no dynamic allocation.
     *
     * @note
     * Calls must be serialized by the application when shared across execution
     * contexts because a measurement temporarily controls both GPIO objects.
     */
    class XWalkUltrasonic
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning trigger-output pointer that is never null after construction. */
            XWalkGpio* triggerPin{nullptr};

            /** @brief Non-owning echo-input pointer that is never null after construction. */
            XWalkGpio* echoPin{nullptr};

            /** @brief Maximum echo acquisition interval in microseconds. */
            uint32 timeoutMicrosecondsValue{XHAL_RPI5CAR_ULTRASONIC_DEFAULT_TIMEOUT_US};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Acquires one ultrasonic distance sample.
             *
             * @return
             * Distance in centimeters rounded to two decimal places;
             * `XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM` on timeout; or
             * `XHAL_RPI5CAR_ULTRASONIC_INVALID_PULSE_RESULT_CM` for an incomplete pulse.
             *
             * @post
             * The trigger output is inactive after a successful GPIO write sequence.
             */
            float64 readOnce();

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a distance sensor from caller-owned GPIO objects.
             *
             * @param[in] trigger
             * GPIO object reconfigured as the trigger output; it must outlive this object.
             *
             * @param[in] echo
             * GPIO object reconfigured as a pull-down echo input; it must outlive this object.
             *
             * @param[in] timeoutMicroseconds
             * Maximum echo acquisition interval in microseconds; zero requests an immediate timeout.
             *
             * @post
             * `trigger` is configured as an output and `echo` as a pull-down input.
             */
            XWalkUltrasonic(XWalkGpio& trigger,
                            XWalkGpio& echo,
                            uint32 timeoutMicroseconds = XHAL_RPI5CAR_ULTRASONIC_DEFAULT_TIMEOUT_US);

            /** @brief Destroys the sensor without releasing its non-owning GPIO dependencies. */
            ~XWalkUltrasonic();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction to preserve dependency identities. */
            XWalkUltrasonic(XWalkUltrasonic&&) = delete;
            /** @brief Disables copying of GPIO dependency bindings. */
            XWalkUltrasonic(const XWalkUltrasonic&) = delete;
            /** @brief Disables move assignment of GPIO dependency bindings. */
            XWalkUltrasonic& operator=(XWalkUltrasonic&&) = delete;
            /** @brief Disables copy assignment of GPIO dependency bindings. */
            XWalkUltrasonic& operator=(const XWalkUltrasonic&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Acquires a distance sample with timeout-only retries.
             *
             * @param[in] attempts
             * Maximum number of measurements; zero performs no GPIO operations.
             *
             * @return
             * First non-timeout result in centimeters, including the invalid-pulse
             * result, or `XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM` when every attempt times out.
             */
            float64 read(uint32 attempts = XHAL_RPI5CAR_ULTRASONIC_DEFAULT_ATTEMPTS);

            /**
             * @brief Cancels interrupt registrations associated with both GPIO dependencies.
             *
             * @post
             * Neither GPIO object retains an interrupt registration managed by `XWalkGpio`.
             */
            void close();

            /**
             * @brief Returns the configured echo acquisition timeout.
             *
             * @return
             * Timeout interval in microseconds.
             */
            uint32 timeoutMicroseconds() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_ULTRASONIC_H */
