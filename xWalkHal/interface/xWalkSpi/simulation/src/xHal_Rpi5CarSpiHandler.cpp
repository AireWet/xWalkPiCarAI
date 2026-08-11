/******************************************************************************
 * @file        xHal_Rpi5CarSpiHandler.cpp
 * @brief       Implements the standalone SPI operation simulation handler.
 *
 * @details
 * Exercises one public SPI transaction without test assertions or test
 * framework dependencies.
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

#include "xHal_Rpi5CarSpiHandler.h"

#include "xHal_Rpi5CarTrace.h"

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkSpi simulation support.
 */
namespace xwalk::hal::sim {

/** @brief Constructs the stateless simulation handler. */
XWalkSpiHandler::XWalkSpiHandler() = default;

/** @brief Destroys the stateless simulation handler. */
XWalkSpiHandler::~XWalkSpiHandler() = default;

/**
 * @brief Runs one JEDEC-identification-style full-duplex transaction.
 * @param[in,out] spi Configured SPI object bound to the selected backend.
 * @return Zero when the response length matches the request; otherwise one.
 */
int32 XWalkSpiHandler::run(XWalkSpi &spi) const {
  const bytevector transmitData{0x9FU, 0x00U, 0x00U, 0x00U};
  const bytevector receivedData = spi.transfer(transmitData);
  XWALK_HAL_TRACE_UID1(RPI .060,
                       "xWalkSpi simulation completed with %zu received bytes",
                       receivedData.size());
  return receivedData.size() == transmitData.size() ? 0 : 1;
}

} /* namespace xwalk::hal::sim */
