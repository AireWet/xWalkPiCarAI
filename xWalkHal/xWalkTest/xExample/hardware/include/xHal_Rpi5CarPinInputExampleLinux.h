/******************************************************************************
 * @file        xHal_Rpi5CarPinInputExampleLinux.h
 * @brief       Declares Linux GPIO composition for the pin-input example.
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

#ifndef XHAL_RPI5CAR_PIN_INPUT_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_PIN_INPUT_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarPinInputExample.h"

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

/** @brief Composes bounded D3 input sampling with Linux GPIO and console I/O. */
class XWalkPinInputExampleLinux final
{
private:

    /** @brief Temporarily bound GPIO valid only during `run()`. */
    XWalkGpio* gpioObject{nullptr};

protected:

    /** @brief Resolves a callback context with one bound GPIO object. */
    static XWalkPinInputExampleLinux& adapter(contextpointer context);
    /** @brief Reads one logical level from the bound D3 input. */
    static boolean read(contextpointer context);
    /** @brief Waits for one bounded sample interval. */
    static void wait(contextpointer context, uint32 durationMilliseconds);
    /** @brief Prints one sampled value as zero or one. */
    static void report(contextpointer context, boolean value);

public:

    /**
     * @brief Samples physical D3 with pull-up at 100-millisecond intervals.
     * @param[in] sampleCount Sample count from one through 36,000.
     * @param[in] gpioDevice Linux GPIO character-device path.
     * @param[in] chipName Optional exact kernel chip name.
     * @param[in] chipLabel Optional exact kernel chip label.
     * @warning Reads physical Robot HAT GPIO22.
     */
    void run(uint32 sampleCount, cstring gpioDevice,
        stringview chipName, stringview chipLabel);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_PIN_INPUT_EXAMPLE_LINUX_H */
