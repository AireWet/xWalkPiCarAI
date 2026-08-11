/******************************************************************************
 * @file        xHal_Rpi5CarI2cDeviceFactoryStub.cpp
 * @brief       Creates the device-free I2C simulation implementation.
 *
 * @details
 * Supplies the host mirror selected by the standalone simulation stub build.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2cDeviceFactory.h"
#include "xHal_Rpi5CarI2cHostStub.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim {

/******************************************************************************
 * Function definitions
 ******************************************************************************/

/** @copydoc createI2cDevice */
owningpointer<XWalkI2cDevice> createI2cDevice() {
  XWALK_HAL_TRACE_UID0(RPI .041, "Creating host-mirror I2C device");
  return std::make_unique<XWalkI2cHostStub>();
}

} /* namespace xwalk::hal::sim */
