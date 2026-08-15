/******************************************************************************
 * @file        xHal_Rpi5CarSpiDeviceFactory.h
 * @brief       Declares the build-selected SPI simulation device factory.
 *
 * @details
 * Exposes one composition function whose implementation is selected by CMake
 * for either device-free simulation or physical Linux SPI hardware.
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

#ifndef XHAL_RPI5CAR_SPI_DEVICE_FACTORY_H
#define XHAL_RPI5CAR_SPI_DEVICE_FACTORY_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpiDevice.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkSpi simulation support.
 */
namespace xwalk::hal::sim
{

    /**
     * @brief Creates the SPI device implementation selected by the build.
     * @return Owned simulation or physical Linux device-operation implementation.
     */
    owningpointer<XWalkSpiDevice> createSpiDevice();

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_SPI_DEVICE_FACTORY_H */
