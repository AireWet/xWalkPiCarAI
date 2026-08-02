/******************************************************************************
 * @file        xAgent_Rpi5CarSpiTransfer.cpp
 * @brief       Implements bounded SPI transfer coordination.
 *
 * @details
 * Binds one non-owning HAL SPI dependency and forwards complete synchronous
 * transactions without owning configuration, device nodes, or chip select.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpiTransfer
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xAgent_Rpi5CarSpiTransfer.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Binds one caller-owned SPI interface.
 * @param[in] spi SPI interface that must outlive this Agent.
 */
XWalkSpiTransfer::XWalkSpiTransfer(hal::XWalkSpi& spi) noexcept:
    spiObject(&spi)
{
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Releases no caller-owned SPI resource. */
XWalkSpiTransfer::~XWalkSpiTransfer() = default;

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Executes one bounded full-duplex transaction.
 * @param[in] transmitData Non-empty payload containing at most 256 bytes.
 * @return Received bytes with exactly the transmitted length.
 */
hal::bytevector XWalkSpiTransfer::transfer(const hal::bytevector& transmitData)
{
    return spiObject->transfer(transmitData);
}

} /* namespace xwalk::agent */
