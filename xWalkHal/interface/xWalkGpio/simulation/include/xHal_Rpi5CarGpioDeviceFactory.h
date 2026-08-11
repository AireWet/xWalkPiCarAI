/******************************************************************************
 * @file        xHal_Rpi5CarGpioDeviceFactory.h
 * @brief       Declares the build-selected GPIO simulation device factory.
 *
 * @details
 * Selects either the device-free host mirror or production Linux adapter at
 * build time without a preprocessor branch in the simulation entry point.
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

#ifndef XHAL_RPI5CAR_GPIO_DEVICE_FACTORY_H
#define XHAL_RPI5CAR_GPIO_DEVICE_FACTORY_H

#include "xHal_Rpi5CarGpioDevice.h"

namespace xwalk::hal::sim
{

/** @brief Creates the GPIO device implementation selected by the build. */
owningpointer<XWalkGpioDevice> createGpioDevice();

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_GPIO_DEVICE_FACTORY_H */
