/******************************************************************************
 * @file        xHal_Rpi5CarRobotHat5MotorSequenceLinux.h
 * @brief       Declares Linux composition for the Robot HAT v5 motor sequence.
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

#ifndef XHAL_RPI5CAR_ROBOTHAT5_MOTOR_SEQUENCE_LINUX_H
#define XHAL_RPI5CAR_ROBOTHAT5_MOTOR_SEQUENCE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarRobotHat5MotorSequence.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::test
{

    /** @brief Composes Robot HAT v5 dual-PWM motors over Linux I2C. */
    class XWalkRobotHat5MotorSequenceLinux
    {
        public:
            /**
             * @brief Runs a bounded physical four-motor sequence.
             *
             * @param[in] i2cDevice
             * Linux I2C character-device path.
             *
             * @param[in] cycleCount
             * Inclusive reverse/forward cycle count from one through 100.
             *
             * @warning
             * This operation physically drives four connected motors in both directions.
             */
            void run(cstring i2cDevice, uint32 cycleCount);

            /** @brief Waits for the requested number of milliseconds. */
            static void wait(contextpointer context, uint32 durationMilliseconds);
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_ROBOTHAT5_MOTOR_SEQUENCE_LINUX_H */
