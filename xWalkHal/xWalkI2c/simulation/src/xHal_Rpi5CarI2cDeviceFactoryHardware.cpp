/******************************************************************************
 * @file        xHal_Rpi5CarI2cDeviceFactoryHardware.cpp
 * @brief       Creates the physical Linux I2C device implementation.
 *
 * @details
 * Supplies the production system-call adapter selected by the standalone
 * simulation hardware build.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Hardware Simulation
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2cDeviceFactory.h"
#include "xHal_Rpi5CarI2cDeviceLinux.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim
{

/******************************************************************************
 * Function definitions
 ******************************************************************************/

/** @copydoc createI2cDevice */
owningpointer<XWalkI2cDevice> createI2cDevice()
{
    XWALK_HAL_TRACE_UID1(RPI.042, "Creating physical Linux I2C device");
    return std::make_unique<XWalkI2cDeviceLinux>();
}

} /* namespace xwalk::hal::sim */
