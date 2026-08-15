/******************************************************************************
 * @file        xHal_Rpi5CarMotorSequenceLinux.h
 * @brief       Declares Linux composition for the two-motor Robot HAT sequence.
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

#ifndef XHAL_RPI5CAR_MOTOR_SEQUENCE_LINUX_H
#define XHAL_RPI5CAR_MOTOR_SEQUENCE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMotorSequence.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::test
{

    /** @brief Composes P13/D4 and P12/D5 motors over Linux I2C and GPIO. */
    class XWalkMotorSequenceLinux
    {
        public:
            /**
             * @brief Runs a bounded physical two-motor sequence.
             *
             * @param[in] i2cDevice
             * Linux I2C character-device path.
             *
             * @param[in] gpioDevice
             * Linux GPIO character-device path.
             *
             * @param[in] chipName
             * Optional expected GPIO chip name.
             *
             * @param[in] chipLabel
             * Optional expected GPIO chip label.
             *
             * @param[in] cycleCount
             * Inclusive reverse/forward cycle count from one through 100.
             *
             * @warning
             * This operation physically drives two connected motors in both directions.
             */
            void
            run(cstring i2cDevice, cstring gpioDevice, stringview chipName, stringview chipLabel, uint32 cycleCount);

            /** @brief Waits for the requested number of milliseconds. */
            static void wait(contextpointer context, uint32 durationMilliseconds);
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_MOTOR_SEQUENCE_LINUX_H */
