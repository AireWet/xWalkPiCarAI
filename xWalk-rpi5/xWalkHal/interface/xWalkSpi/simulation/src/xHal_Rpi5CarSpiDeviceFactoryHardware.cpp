/******************************************************************************
 * @file        xHal_Rpi5CarSpiDeviceFactoryHardware.cpp
 * @brief       Creates the physical Linux SPI device implementation.
 *
 * @details
 * Supplies the production system-call adapter selected by the standalone
 * simulation hardware build.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Hardware Simulation
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
#include "xHal_Rpi5CarSpiDeviceLinux.h"
#include "xHal_Rpi5CarTrace.h"

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkSpi simulation support.
 */
namespace xwalk::hal::sim {

/** @copydoc createSpiDevice */
owningpointer<XWalkSpiDevice> createSpiDevice() {
  XWALK_HAL_TRACE_UID0(RPI .058, "Creating physical Linux SPI device");
  return std::make_unique<XWalkSpiDeviceLinux>();
}

} /* namespace xwalk::hal::sim */
