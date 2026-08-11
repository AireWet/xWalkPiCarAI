/******************************************************************************
 * @file        xHal_Rpi5CarSpiDeviceFactoryStub.cpp
 * @brief       Creates the device-free SPI simulation implementation.
 *
 * @details
 * Supplies the host mirror selected by the standalone simulation stub build.
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

#include "xHal_Rpi5CarSpiDeviceFactory.h"
#include "xHal_Rpi5CarSpiHostStub.h"
#include "xHal_Rpi5CarTrace.h"

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkSpi simulation support.
 */
namespace xwalk::hal::sim {

/** @copydoc createSpiDevice */
owningpointer<XWalkSpiDevice> createSpiDevice() {
  XWALK_HAL_TRACE_UID0(RPI .057, "Creating host-mirror SPI device");
  return std::make_unique<XWalkSpiHostStub>();
}

} /* namespace xwalk::hal::sim */
