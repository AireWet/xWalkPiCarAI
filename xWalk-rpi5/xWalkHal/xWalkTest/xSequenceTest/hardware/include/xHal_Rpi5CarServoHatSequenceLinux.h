/******************************************************************************
 * @file        xHal_Rpi5CarServoHatSequenceLinux.h
 * @brief       Declares Linux composition for the Robot HAT servo sequence.
 *
 * @details
 * Exposes physical Linux I2C and GPIO composition plus trace and timing
 * callbacks for the bounded servo sweep and ADC monitor.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_SERVO_HAT_SEQUENCE_LINUX_H
#define XHAL_RPI5CAR_SERVO_HAT_SEQUENCE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServoHatSequence.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @brief Composes and reports the physical Robot HAT servo and ADC sequence.
     *
     * @details
     * Owns no persistent hardware state. Each run creates its Linux backends and
     * HAL objects in dependency order and releases them before returning.
     */
    class XWalkServoHatSequenceLinux
    {
        public:
            /**
             * @brief Runs the bounded physical servo sweep and ADC monitor.
             *
             * @param[in] i2cDevice
             * Linux I2C character-device path.
             *
             * @param[in] gpioDevice
             * Linux GPIO chip-device path.
             *
             * @param[in] gpioChipName
             * Optional exact GPIO chip name.
             *
             * @param[in] gpioChipLabel
             * Optional exact GPIO chip label.
             *
             * @param[in] sampleCount
             * ADC sample count in the inclusive range 1 through 3600.
             *
             * @warning
             * This operation resets the MCU and physically moves all 16 servo channels.
             */
            void
            run(cstring i2cDevice, cstring gpioDevice, cstring gpioChipName, cstring gpioChipLabel, uint32 sampleCount);

            /**
             * @brief Waits for the requested duration.
             *
             * @param[in,out] context
             * Unused callback context.
             *
             * @param[in] durationMilliseconds
             * Requested wait duration in milliseconds.
             */
            static void wait(contextpointer context, uint32 durationMilliseconds);

            /**
             * @brief Traces the upstream servo-channel status message.
             *
             * @param[in,out] context
             * Unused callback context.
             *
             * @param[in] channel
             * Servo PWM channel in the range zero through 15.
             */
            static void reportServo(contextpointer context, uint8 channel);

            /**
             * @brief Traces one ordered five-channel ADC sample.
             *
             * @param[in,out] context
             * Unused callback context.
             *
             * @param[in] readings
             * Raw ADC readings ordered by channels zero through four.
             */
            static void reportAdc(contextpointer context, const servohatreadings& readings);
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_SERVO_HAT_SEQUENCE_LINUX_H */
