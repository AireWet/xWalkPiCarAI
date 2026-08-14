/******************************************************************************
 * @file        xHal_Rpi5CarI2cHandler.h
 * @brief       Declares the standalone I2C operation simulation handler.
 *
 * @details
 * Runs the normal public I2C operations against whichever Linux backend device
 * implementation was selected by the build.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-09
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_I2C_HANDLER_H
#define XHAL_RPI5CAR_I2C_HANDLER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2c.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::sim
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/** @brief Executes one representative sequence through the public I2C API. */
class XWalkI2cHandler final
{
    public:
        /** @brief Constructs the stateless simulation handler. */
        XWalkI2cHandler();

        /** @brief Destroys the stateless simulation handler. */
        ~XWalkI2cHandler();

        XWalkI2cHandler(XWalkI2cHandler&&) = delete;
        XWalkI2cHandler(const XWalkI2cHandler&) = delete;
        XWalkI2cHandler& operator=(XWalkI2cHandler&&) = delete;
        XWalkI2cHandler& operator=(const XWalkI2cHandler&) = delete;

        /**
         * @brief Runs probe, write, safe-write, read, and register-read operations.
         * @param[in,out] i2c Configured I2C object bound to the selected backend.
         * @return Zero when the address responds and every operation completes;
         * otherwise a non-zero status.
         */
        int32 run(XWalkI2c& i2c) const;
};

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_I2C_HANDLER_H */
