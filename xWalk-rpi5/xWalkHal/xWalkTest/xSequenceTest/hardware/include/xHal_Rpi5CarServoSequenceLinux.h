/******************************************************************************
 * @file        xHal_Rpi5CarServoSequenceLinux.h
 * @brief       Declares Linux composition for the 12-channel servo sequence.
 *
 * @details
 * Exposes physical Robot HAT I2C composition and timing for the bounded servo
 * sequence without retaining hardware resources between runs.
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

#ifndef XHAL_RPI5CAR_SERVO_SEQUENCE_LINUX_H
#define XHAL_RPI5CAR_SERVO_SEQUENCE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServoSequence.h"

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

    /** @brief Composes the physical Robot HAT 12-channel servo sequence. */
    class XWalkServoSequenceLinux
    {
        public:
            /**
             * @brief Runs bounded physical negative and positive servo sweeps.
             *
             * @param[in] i2cDevice
             * Linux I2C character-device path.
             *
             * @param[in] cycleCount
             * Complete sweep count in the inclusive range one through 100.
             *
             * @warning
             * This operation physically moves servo channels zero through 11.
             */
            void run(cstring i2cDevice, uint32 cycleCount);

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
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_SERVO_SEQUENCE_LINUX_H */
