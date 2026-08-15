/******************************************************************************
 * @file        xHal_Rpi5CarI2cDeviceFactory.h
 * @brief       Declares the build-selected I2C simulation device factory.
 *
 * @details
 * Exposes one composition function whose implementation is selected by CMake
 * for either device-free simulation or physical Linux I2C hardware.
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

#ifndef XHAL_RPI5CAR_I2C_DEVICE_FACTORY_H
#define XHAL_RPI5CAR_I2C_DEVICE_FACTORY_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2cDevice.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::sim
{

    /******************************************************************************
     * Function declarations
     ******************************************************************************/

    /**
     * @brief Creates the I2C device implementation selected by the build.
     * @return Owned simulation or physical Linux device-operation implementation.
     */
    owningpointer<XWalkI2cDevice> createI2cDevice();

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_I2C_DEVICE_FACTORY_H */
