/******************************************************************************
 * @file        xHal_Rpi5CarLedExampleLinux.cpp
 * @brief       Implements Linux composition for the Robot HAT LED example.
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

#include "xHal_Rpi5CarLedExampleLinux.h"

#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarLed.h"

#include "xHal_Rpi5CarTrace.h"
#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::example
{

    /**
     * @brief Runs the physical LED example through GPIO26.
     *
     * @param[in] gpioDevice Linux GPIO character-device path.
     * @param[in] chipName Optional exact kernel chip name.
     * @param[in] chipLabel Optional exact kernel chip label.
     */
    void XWalkLedExampleLinux::run(cstring gpioDevice, stringview chipName, stringview chipLabel)
    {
        XWalkGpioLinux backend(gpioDevice, chipName, chipLabel, 27U);
        const XWalkGpioCallbacks gpioCallbacks = XHAL_GPIO_CALLBACKS(XWalkGpioLinux);
        XWalkGpio gpio(&backend, gpioCallbacks, "LED");
        XWalkLed led(gpio);
        ledObject = &led;
        const XWalkLedExampleCallbacks exampleCallbacks{&on, &off, &blink, &close, &wait, &report};
        XWalkLedExample example(this, exampleCallbacks);
        try
        {
            example.run();
            ledObject = nullptr;
        }
        catch (...)
        {
            ledObject = nullptr;
            throw;
        }
    }

    /**
     * @brief Converts one callback context into its required Linux adapter.
     *
     * @param[in,out] context Non-null adapter context with one bound LED.
     * @return Referenced adapter.
     * @throws std::invalid_argument If the context or bound LED is null.
     */
    XWalkLedExampleLinux& XWalkLedExampleLinux::adapter(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "LED example Linux context must not be null");
        }
        XWalkLedExampleLinux& self = *static_cast<XWalkLedExampleLinux*>(context);
        if (self.ledObject == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "LED example Linux adapter has no bound LED");
        }
        return self;
    }

    /** @brief Activates the bound LED. */
    void XWalkLedExampleLinux::on(contextpointer context)
    {
        adapter(context).ledObject->on();
    }

    /** @brief Deactivates the bound LED. */
    void XWalkLedExampleLinux::off(contextpointer context)
    {
        adapter(context).ledObject->off();
    }

    /**
     * @brief Starts one background blink configuration.
     *
     * @param[in,out] context Non-null Linux adapter context.
     * @param[in] cycleCount Complete on/off cycles per repeated sequence.
     * @param[in] toggleDelaySeconds Delay between transitions in seconds.
     * @param[in] pauseSeconds Inactive delay after each repeated sequence.
     */
    void XWalkLedExampleLinux::blink(contextpointer context,
                                     uint32 cycleCount,
                                     float64 toggleDelaySeconds,
                                     float64 pauseSeconds)
    {
        adapter(context).ledObject->blink(cycleCount, toggleDelaySeconds, pauseSeconds);
    }

    /** @brief Stops and closes the bound LED. */
    void XWalkLedExampleLinux::close(contextpointer context)
    {
        adapter(context).ledObject->close();
    }

    /**
     * @brief Waits for one source duration.
     *
     * @param[in,out] context Non-null Linux adapter context.
     * @param[in] durationMilliseconds Requested duration in milliseconds.
     */
    void XWalkLedExampleLinux::wait(contextpointer context, uint32 durationMilliseconds)
    {
        static_cast<void>(adapter(context));
        common::sleepMilliseconds(durationMilliseconds);
    }

    /**
     * @brief Prints one source-compatible progress message.
     *
     * @param[in,out] context Non-null Linux adapter context.
     * @param[in] message Status message copied from the Python example.
     */
    void XWalkLedExampleLinux::report(contextpointer context, stringview message)
    {
        static_cast<void>(adapter(context));
        std::cout << message << '\n';
    }

} /* namespace xwalk::hal::example */
