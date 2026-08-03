/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicExampleLinux.h
 * @brief       Declares Linux GPIO composition for the ultrasonic example.
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

#ifndef XHAL_RPI5CAR_ULTRASONIC_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_ULTRASONIC_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarUltrasonic.h"
#include "xHal_Rpi5CarUltrasonicExample.h"

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

/** @brief Composes bounded D2/D3 ranging with Linux GPIO and console I/O. */
class XWalkUltrasonicExampleLinux final
{
private:

    /** @brief Temporarily bound sensor valid only during `run()`. */
    XWalkUltrasonic* sensorObject{nullptr};

protected:

    /** @brief Resolves a callback context with one bound ultrasonic sensor. */
    static XWalkUltrasonicExampleLinux& adapter(contextpointer context);
    /** @brief Reads one distance in centimeters from the bound sensor. */
    static float64 read(contextpointer context);
    /** @brief Waits for one bounded sample interval. */
    static void wait(contextpointer context, uint32 durationMilliseconds);
    /** @brief Prints one source-compatible distance in centimeters. */
    static void report(contextpointer context, float64 distanceCentimeters);

public:

    /**
     * @brief Samples physical D2/D3 ranging at 200-millisecond intervals.
     * @param[in] sampleCount Sample count from one through 18,000.
     * @param[in] gpioDevice Linux GPIO character-device path.
     * @param[in] chipName Optional exact kernel chip name.
     * @param[in] chipLabel Optional exact kernel chip label.
     * @warning Pulses Robot HAT GPIO27 and reads physical GPIO22.
     */
    void run(uint32 sampleCount, cstring gpioDevice,
        stringview chipName, stringview chipLabel);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_ULTRASONIC_EXAMPLE_LINUX_H */
