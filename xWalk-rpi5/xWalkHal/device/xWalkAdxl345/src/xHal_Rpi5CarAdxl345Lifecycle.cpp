/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345Lifecycle.cpp
 * @brief       Implements ADXL345 validation and lifecycle behavior.
 *
 * @details
 * Validates axis and address inputs, maps axes to data registers, and binds a
 * caller-owned I2C interface without taking ownership.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAdxl345.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates an axis selector.
 *
 * @param[in] axis
 * Axis expected to be X, Y, or Z.
 *
 * @return
 * Validated zero-based axis index.
 *
 * @throws std::out_of_range
 * If `axis` does not identify X, Y, or Z.
 */
uint8 XWalkAdxl345::validateAxis(XWalkAdxl345Axis axis) {
  const uint8 axisValue = static_cast<uint8>(axis);
  if (axisValue >= XHAL_RPI5CAR_ADXL345_AXIS_COUNT) {
    XWALK_HAL_ERROR(XWALK_RANGE, "ADXL345 axis must be X, Y, or Z");
  }
  return axisValue;
}

/**
 * @brief Returns the first data register for an axis.
 *
 * @param[in] axis
 * Validated zero-based axis index.
 *
 * @return
 * Low-byte data register for the selected axis.
 *
 * @pre
 * `axis` is less than three.
 */
uint8 XWalkAdxl345::registerForAxis(uint8 axis) noexcept {
  if (axis == XHAL_RPI5CAR_ADXL345_X_AXIS) {
    return XHAL_RPI5CAR_ADXL345_DATA_X_REGISTER;
  }
  if (axis == XHAL_RPI5CAR_ADXL345_Y_AXIS) {
    return XHAL_RPI5CAR_ADXL345_DATA_Y_REGISTER;
  }
  return XHAL_RPI5CAR_ADXL345_DATA_Z_REGISTER;
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs an ADXL345 interface.
 *
 * @param[in] i2c
 * I2C dependency that must outlive this object and provide register reads.
 *
 * @param[in] address
 * Seven-bit ADXL345 address; defaults to `0x53`.
 *
 * @throws std::out_of_range
 * If `address` exceeds the seven-bit I2C range.
 */
XWalkAdxl345::XWalkAdxl345(XWalkI2c &i2c, uint8 address)
    : i2cObject(&i2c), addressValue(address) {
  common::validateI2cAddress(addressValue);
  XWALK_HAL_TRACE_UID1(RPI .192, "ADXL345 constructed at address 0x%02X",
                       addressValue);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the interface without releasing its non-owning I2C
 * dependency.
 */
XWalkAdxl345::~XWalkAdxl345() = default;

} /* namespace xwalk::hal */
