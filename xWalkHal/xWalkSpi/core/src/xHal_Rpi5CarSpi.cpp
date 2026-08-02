/******************************************************************************
 * @file        xHal_Rpi5CarSpi.cpp
 * @brief       Implements bounded hardware-independent SPI transfers.
 *
 * @details
 * Validates payload bounds, forwards one synchronous full-duplex transaction,
 * and rejects backend responses whose length differs from the request.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi
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

#include "xHal_Rpi5CarSpi.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Performs one bounded full-duplex transaction.
 * @param[in] transmitData Non-empty payload containing at most 256 bytes.
 * @return Received bytes with exactly the transmitted length.
 * @throws std::invalid_argument If the payload is empty.
 * @throws std::out_of_range If the payload exceeds 256 bytes.
 * @throws std::runtime_error If the backend returns an unexpected length.
 */
bytevector XWalkSpi::transfer(const bytevector& transmitData)
{
    if (transmitData.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("SPI transfer payload must not be empty");
    }
    if (transmitData.size() > XHAL_RPI5CAR_SPI_MAXIMUM_TRANSFER_BYTES)
    {
        XHAL_THROW_OUT_OF_RANGE("SPI transfer payload exceeds 256 bytes");
    }

    bytevector receivedData = transferCallback(contextValue, transmitData);
    if (receivedData.size() != transmitData.size())
    {
        XHAL_THROW_RUNTIME_ERROR("SPI backend returned an unexpected byte count");
    }
    return receivedData;
}

} /* namespace xwalk::hal */
