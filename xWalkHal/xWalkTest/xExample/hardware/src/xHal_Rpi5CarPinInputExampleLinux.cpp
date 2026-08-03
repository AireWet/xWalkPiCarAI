/******************************************************************************
 * @file        xHal_Rpi5CarPinInputExampleLinux.cpp
 * @brief       Implements Linux GPIO composition for the pin-input example.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarPinInputExampleLinux.h"

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarGpioLinux.h"

#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains Linux adapters for ported example programs.
 */
namespace xwalk::hal::example
{

/**
 * @brief Runs bounded physical D3 input sampling.
 * @param[in] sampleCount Sample count from one through 36,000.
 * @param[in] gpioDevice Linux GPIO character-device path.
 * @param[in] chipName Optional exact kernel chip name.
 * @param[in] chipLabel Optional exact kernel chip label.
 * @warning Reads physical Robot HAT GPIO22.
 */
void XWalkPinInputExampleLinux::run(uint32 sampleCount, cstring gpioDevice,
    stringview chipName, stringview chipLabel)
{
    XWalkGpioLinux backend(gpioDevice, chipName, chipLabel, 22U);
    const XWalkGpioCallbacks gpioCallbacks = XHAL_GPIO_CALLBACKS(XWalkGpioLinux);
    XWalkGpio gpio(&backend, gpioCallbacks, "D3",
        XWalkGpioMode::Input, XWalkGpioPull::Up);
    gpioObject = &gpio;
    const XWalkPinInputExampleCallbacks exampleCallbacks{
        &read, &wait, &report};
    XWalkPinInputExample example(this, exampleCallbacks);
    try
    {
        example.run(sampleCount);
        gpioObject = nullptr;
    }
    catch (...)
    {
        gpioObject = nullptr;
        throw;
    }
}

/**
 * @brief Resolves one callback context with a bound GPIO object.
 * @param[in,out] context Non-null Linux adapter context.
 * @return Referenced adapter.
 * @throws std::invalid_argument If the adapter or GPIO binding is invalid.
 */
XWalkPinInputExampleLinux& XWalkPinInputExampleLinux::adapter(
    contextpointer context)
{
    if (context == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Pin-input Linux context must not be null");
    }
    XWalkPinInputExampleLinux& self =
        *static_cast<XWalkPinInputExampleLinux*>(context);
    if (self.gpioObject == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Pin-input Linux adapter has no bound GPIO");
    }
    return self;
}

/** @brief Reads one logical value from the bound D3 input. */
boolean XWalkPinInputExampleLinux::read(contextpointer context)
{
    return adapter(context).gpioObject->read();
}

/**
 * @brief Waits for one source-compatible sample interval.
 * @param[in,out] context Non-null Linux adapter context.
 * @param[in] durationMilliseconds Requested duration in milliseconds.
 */
void XWalkPinInputExampleLinux::wait(
    contextpointer context, uint32 durationMilliseconds)
{
    static_cast<void>(adapter(context));
    common::sleepMilliseconds(durationMilliseconds);
}

/**
 * @brief Prints one source-compatible logical value.
 * @param[in,out] context Non-null Linux adapter context.
 * @param[in] value Logical value to print as zero or one.
 */
void XWalkPinInputExampleLinux::report(
    contextpointer context, boolean value)
{
    static_cast<void>(adapter(context));
    std::cout << (value ? 1U : 0U) << '\n';
}

} /* namespace xwalk::hal::example */
