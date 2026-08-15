/******************************************************************************
 * @file        xHal_Rpi5CarLedLifecycle.cpp
 * @brief       Implements LED construction and destruction.
 *
 * @details
 * Binds the caller-owned GPIO, establishes the inactive output state, and
 * guarantees that a joinable blink worker is stopped before destruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkLed
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarLed.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs an inactive LED controller.
     *
     * @param[in] gpio
     * Non-owning GPIO output whose logical polarity represents LED state.
     *
     * @pre
     * `gpio` outlives this object and any blink operation.
     *
     * @post
     * The GPIO output is logically inactive.
     */
    XWalkLed::XWalkLed(XWalkGpio& gpio) : gpioObject(&gpio)
    {
        static_cast<void>(gpioObject->off());
        outputValue.store(false);
        XWALK_HAL_TRACE_UID0(RPI .258, "GPIO-backed LED constructed in the inactive state");
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Stops the blink worker before destroying the LED controller.
     *
     * @note
     * The non-owning GPIO is not released.
     */
    XWalkLed::~XWalkLed()
    {
        stopWorker();
    }

} /* namespace xwalk::hal */
