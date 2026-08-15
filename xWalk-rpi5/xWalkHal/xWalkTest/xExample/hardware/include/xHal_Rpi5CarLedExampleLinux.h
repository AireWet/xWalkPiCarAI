/******************************************************************************
 * @file        xHal_Rpi5CarLedExampleLinux.h
 * @brief       Declares Linux composition for the Robot HAT LED example.
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

#ifndef XHAL_RPI5CAR_LED_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_LED_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLedExample.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal
{
    class XWalkLed;
}

/**
 * @namespace xwalk::hal::example
 * @brief Contains Linux composition for ported upstream examples.
 */
namespace xwalk::hal::example
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Composes the LED example with Linux GPIO and the real LED controller. */
    class XWalkLedExampleLinux
    {
        private:
            /** @brief Non-owning active LED pointer valid only while `run()` executes. */
            XWalkLed* ledObject{nullptr};

        protected:
            /** @brief Converts a callback context into its required Linux adapter. */
            static XWalkLedExampleLinux& adapter(contextpointer context);
            /** @brief Activates the bound LED. */
            static void on(contextpointer context);
            /** @brief Deactivates the bound LED. */
            static void off(contextpointer context);
            /** @brief Starts the requested background blink configuration. */
            static void
            blink(contextpointer context, uint32 cycleCount, float64 toggleDelaySeconds, float64 pauseSeconds);
            /** @brief Stops and closes the bound LED. */
            static void close(contextpointer context);
            /** @brief Waits for the requested source duration. */
            static void wait(contextpointer context, uint32 durationMilliseconds);
            /** @brief Prints one source-compatible progress message. */
            static void report(contextpointer context, stringview message);

        public:
            /**
             * @brief Runs the example on the Robot HAT LED GPIO.
             *
             * @param[in] gpioDevice Linux GPIO character-device path.
             * @param[in] chipName Optional exact kernel GPIO chip name.
             * @param[in] chipLabel Optional exact kernel GPIO chip label.
             *
             * @warning Changes physical GPIO26 for approximately 19 seconds.
             */
            void run(cstring gpioDevice, stringview chipName, stringview chipLabel);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_LED_EXAMPLE_LINUX_H */
