/******************************************************************************
 * @file        xHal_Rpi5CarServoExampleLinux.h
 * @brief       Declares Linux I2C composition for the servo example.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
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

#ifndef XHAL_RPI5CAR_SERVO_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_SERVO_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServo.h"
#include "xHal_Rpi5CarServoExample.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains Linux adapters for ported example programs.
 */
namespace xwalk::hal::example
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/** @brief Composes channel-one sweeps with Linux I2C and console output. */
class XWalkServoExampleLinux final
{
private:

    XWalkServo* servoObject{nullptr};

protected:

    static XWalkServoExampleLinux& adapter(contextpointer context);
    static void setAngle(contextpointer context, float64 angleDegrees);
    static void wait(contextpointer context, uint32 durationMilliseconds);
    static void report(contextpointer context, int32 angleDegrees);

public:

    /**
     * @brief Runs bounded physical sweeps on Robot HAT servo channel one.
     * @param[in] i2cDevice Linux I2C character-device path.
     * @param[in] cycleCount Complete cycles from one through 100.
     * @warning Physically moves the servo through its full supported range.
     */
    void run(cstring i2cDevice, uint32 cycleCount);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_SERVO_EXAMPLE_LINUX_H */
