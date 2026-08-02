/******************************************************************************
 * @file        xHal_Rpi5CarSpiTypes.h
 * @brief       Declares xWalk SPI configuration and callback types.
 *
 * @details
 * Defines bounded full-duplex transfer settings and the non-owning callback
 * contract shared by the hardware-independent SPI interface and its backends.
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

#ifndef XHAL_RPI5CAR_SPI_TYPES_H
#define XHAL_RPI5CAR_SPI_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @struct XWalkSpiConfiguration
 * @brief Stores Linux-compatible SPI clock and word settings.
 */
struct XWalkSpiConfiguration
{
    /** @brief Clock frequency in Hertz; zero is invalid. */
    uint32 speedHz{XHAL_RPI5CAR_SPI_DEFAULT_SPEED_HZ};
    /** @brief Standard SPI mode in the inclusive range zero through three. */
    uint8 mode{XHAL_RPI5CAR_SPI_DEFAULT_MODE};
    /** @brief Bits per word in the inclusive range one through thirty-two. */
    uint8 bitsPerWord{XHAL_RPI5CAR_SPI_DEFAULT_BITS_PER_WORD};
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Performs one bounded full-duplex SPI transaction.
 *
 * @param[in,out] context
 * Nullable non-owning backend context that remains valid for the callback.
 *
 * @param[in] transmitData
 * Non-empty payload containing at most 256 bytes in wire order.
 *
 * @return
 * Received bytes with exactly the same length and wire order as the request.
 */
using spitransfercallback = bytevector (*)(contextpointer context,
    const bytevector& transmitData);

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPI_TYPES_H */
