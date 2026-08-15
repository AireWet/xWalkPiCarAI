/******************************************************************************
 * @file        xHal_Rpi5CarGpioHandler.h
 * @brief       Declares the standalone GPIO operation simulation handler.
 *
 * @details
 * Runs representative public GPIO output and input operations against the
 * Linux backend device implementation selected by the build.
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

#ifndef XHAL_RPI5CAR_GPIO_HANDLER_H
#define XHAL_RPI5CAR_GPIO_HANDLER_H

#include "xHal_Rpi5CarGpio.h"

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkGpio simulation support.
 */
namespace xwalk::hal::sim
{

    /** @brief Executes representative operations through the public GPIO API. */
    class XWalkGpioHandler final
    {
        public:
            /** @brief Constructs the stateless simulation handler. */
            XWalkGpioHandler();

            /** @brief Destroys the stateless simulation handler. */
            ~XWalkGpioHandler();

            XWalkGpioHandler(XWalkGpioHandler&&) = delete;
            XWalkGpioHandler(const XWalkGpioHandler&) = delete;
            XWalkGpioHandler& operator=(XWalkGpioHandler&&) = delete;
            XWalkGpioHandler& operator=(const XWalkGpioHandler&) = delete;

            /**
             * @brief Drives the selected line low and then samples its level.
             * @param[in,out] gpio Configured GPIO object bound to the selected backend.
             * @return Zero after the operations complete.
             */
            int32 run(XWalkGpio& gpio) const;
    };

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_GPIO_HANDLER_H */
