/******************************************************************************
 * @file        xHal_Rpi5CarSpiHandler.h
 * @brief       Declares the standalone SPI operation simulation handler.
 *
 * @details
 * Runs a representative public SPI transaction against the Linux backend
 * device implementation selected by the build.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Simulation
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

#ifndef XHAL_RPI5CAR_SPI_HANDLER_H
#define XHAL_RPI5CAR_SPI_HANDLER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpi.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkSpi simulation support.
 */
namespace xwalk::hal::sim
{

/** @brief Executes one representative transaction through the public SPI API. */
class XWalkSpiHandler final
{
    public:
        /** @brief Constructs the stateless simulation handler. */
        XWalkSpiHandler();

        /** @brief Destroys the stateless simulation handler. */
        ~XWalkSpiHandler();

        XWalkSpiHandler(XWalkSpiHandler&&) = delete;
        XWalkSpiHandler(const XWalkSpiHandler&) = delete;
        XWalkSpiHandler& operator=(XWalkSpiHandler&&) = delete;
        XWalkSpiHandler& operator=(const XWalkSpiHandler&) = delete;

        /**
         * @brief Runs one JEDEC-identification-style full-duplex transaction.
         * @param[in,out] spi Configured SPI object bound to the selected backend.
         * @return Zero when the response length matches the request.
         */
        int32 run(XWalkSpi& spi) const;
};

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_SPI_HANDLER_H */
