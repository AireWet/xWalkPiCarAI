/******************************************************************************
 * @file        xHal_Rpi5CarButtonEventSequenceLinux.h
 * @brief       Declares Linux callbacks for the D0 button-event sequence.
 *
 * @details
 * Supplies wall-clock timestamps, bounded sleeping, and trace event output
 * while keeping the sequence core independent from Linux services.
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

#ifndef XHAL_RPI5CAR_BUTTON_EVENT_SEQUENCE_LINUX_H
#define XHAL_RPI5CAR_BUTTON_EVENT_SEQUENCE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::test
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Provides Linux operations injected into the sequence core. */
    class XWalkButtonEventSequenceLinux
    {
        public:
            /**
             * @brief Runs the core sequence with Linux timing and event output.
             *
             * @param[in] gpio
             * Caller-owned D0 GPIO backed by the Linux implementation.
             *
             * @param[in] durationSeconds
             * Inclusive monitoring duration from one through 3600 seconds.
             */
            void run(XWalkGpio& gpio, uint32 durationSeconds);

            /** @brief Announces a physical monitoring interval. */
            void announce(uint32 durationSeconds) const;

            /** @brief Waits for the requested number of milliseconds. */
            static void wait(contextpointer context, uint32 durationMilliseconds);

            /** @brief Returns wall-clock seconds since the Unix epoch. */
            static float64 time(contextpointer context);

            /** @brief Traces one timestamped button event. */
            static void event(contextpointer context, boolean pressed, float64 timestampSeconds);
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_BUTTON_EVENT_SEQUENCE_LINUX_H */
