/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345Types.h
 * @brief       Declares ADXL345 axis and aggregate value types.
 *
 * @details
 * Defines the strongly typed axis selector and fixed three-axis acceleration
 * result used by the xWalkAdxl345 public interface.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdxl345
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_ADXL345_TYPES_H
#define XHAL_RPI5CAR_ADXL345_TYPES_H

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
 * Enumeration declarations
 ******************************************************************************/

/** @brief Selects one orthogonal ADXL345 acceleration axis. */
enum class XWalkAdxl345Axis : uint8
{
    X = XHAL_RPI5CAR_ADXL345_X_AXIS, /**< X-axis acceleration. */
    Y = XHAL_RPI5CAR_ADXL345_Y_AXIS, /**< Y-axis acceleration. */
    Z = XHAL_RPI5CAR_ADXL345_Z_AXIS  /**< Z-axis acceleration. */
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Fixed X-, Y-, and Z-axis acceleration values in standard gravity. */
using adxl345values = fixedarray<float64, XHAL_RPI5CAR_ADXL345_AXIS_COUNT>;

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_ADXL345_TYPES_H */
