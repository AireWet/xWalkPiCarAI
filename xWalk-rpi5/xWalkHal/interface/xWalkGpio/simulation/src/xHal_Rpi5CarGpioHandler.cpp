/******************************************************************************
 * @file        xHal_Rpi5CarGpioHandler.cpp
 * @brief       Implements the standalone GPIO operation simulation handler.
 *
 * @details
 * Exercises public GPIO output and input operations without test assertions or
 * test-framework dependencies.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarGpioHandler.h"

#include "xHal_Rpi5CarTrace.h"

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkGpio simulation
 * support.
 */
namespace xwalk::hal::sim
{

    /** @brief Constructs the stateless simulation handler. */
    XWalkGpioHandler::XWalkGpioHandler() = default;

    /** @brief Destroys the stateless simulation handler. */
    XWalkGpioHandler::~XWalkGpioHandler() = default;

    /**
     * @brief Drives the selected line low and then samples its level.
     * @param[in,out] gpio Configured GPIO object bound to the selected backend.
     * @return Zero after the operations complete.
     */
    int32 XWalkGpioHandler::run(XWalkGpio& gpio) const
    {
        const boolean outputValue = gpio.off();
        const boolean inputValue = gpio.read();
        XWALK_HAL_TRACE_UID2(RPI .081,
                             "xWalkGpio simulation completed with output %d and input %d",
                             static_cast<int32>(outputValue),
                             static_cast<int32>(inputValue));
        return 0;
    }

} /* namespace xwalk::hal::sim */
