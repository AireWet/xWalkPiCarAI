/******************************************************************************
 * @file        xHal_Rpi5CarInitAnglesSequenceLinux.h
 * @brief       Declares Linux composition for the init-angles sequence.
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

#ifndef XHAL_RPI5CAR_INIT_ANGLES_SEQUENCE_LINUX_H
#define XHAL_RPI5CAR_INIT_ANGLES_SEQUENCE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::test
{

/** @brief Composes Linux GPIO/I2C dependencies for physical servo movement. */
class XWalkInitAnglesSequenceLinux
{
public:

    /**
     * @brief Resets the Robot HAT and initializes PWM channels 10, 11, and 12.
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
     * @param[in] configurationPath
     * Writable sequence-specific calibration file.
     *
     * @warning
     * This operation resets the MCU and physically moves three servos.
     */
    void run(cstring i2cDevice, cstring gpioDevice, cstring gpioChipName,
        cstring gpioChipLabel, stringview configurationPath);
};

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_INIT_ANGLES_SEQUENCE_LINUX_H */
