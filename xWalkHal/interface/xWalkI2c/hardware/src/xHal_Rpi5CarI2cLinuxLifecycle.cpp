/******************************************************************************
 * @file        xHal_Rpi5CarI2cLinuxLifecycle.cpp
 * @brief       Implements Linux I2C backend validation and lifecycle behavior.
 *
 * @details
 * Validates retry configuration, opens the Linux I2C device with xWalk
 * lifecycle diagnostics, and closes the owned descriptor during destruction.
 * Callback composition remains outside the backend.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Linux Backend
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

#include "xHal_Rpi5CarI2cDeviceLinux.h"
#include "xHal_Rpi5CarI2cLinux.h"

#include "xHal_Rpi5CarCommon.h"
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
 * Anonymous namespace
 ******************************************************************************/

namespace {

/** @brief Returns the process-lifetime production device interface. */
XWalkI2cDevice &systemDeviceInterface() {
  static XWalkI2cDeviceLinux deviceInterface;
  return deviceInterface;
}

} /* namespace */

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates the configured I2C operation attempt count.
 *
 * @param[in] retryCount
 * Number of permitted attempts.
 *
 * @return
 * The validated non-zero attempt count.
 *
 * @throws std::invalid_argument
 * If `retryCount` is zero.
 */
uint32 XWalkI2cLinux::validateRetryCount(uint32 retryCount) {
  if (retryCount == 0U) {
    XWALK_HAL_ERROR(XWALK_INVAL, "I2C retry count must be greater than zero");
  }
  XWALK_HAL_TRACE_UID1(RPI .025, "Linux I2C retry count validated: %u",
                       retryCount);
  return retryCount;
}

/**
 * @brief Opens a Linux I2C device node for owned read/write access.
 *
 * @param[in] devicePath
 * Non-null path to the I2C device node.
 *
 * @return
 * Owned non-negative Linux file descriptor.
 *
 * @throws std::invalid_argument
 * If `devicePath` is null.
 *
 * @throws std::runtime_error
 * If the device cannot be opened.
 */
int32 XWalkI2cLinux::openDevice(cstring devicePath) {
  if (devicePath == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "I2C device path must not be null");
  }

  XWALK_HAL_TRACE_UID1(RPI .026, "Opening Linux I2C device: %s", devicePath);
  const int32 descriptor = deviceInterfaceValue.openDevice(devicePath);
  if (descriptor < 0) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Unable to open Linux I2C device");
  }
  XWALK_HAL_TRACE_UID1(RPI .027, "Linux I2C device opened with descriptor %d",
                       descriptor);
  return descriptor;
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a Linux I2C backend and opens its device node.
 *
 * @param[in] devicePath
 * Non-null path to the Linux I2C device node.
 *
 * @param[in] retryCount
 * Number of probe, write, or read attempts; valid range is 1 to `UINT32_MAX`.
 *
 * @post
 * On successful construction, the backend owns an open descriptor and is ready
 * to receive calls from an externally created callback interface.
 *
 * @throws std::invalid_argument
 * If `devicePath` is null or `retryCount` is zero.
 *
 * @throws std::runtime_error
 * If the Linux device node cannot be opened.
 */
XWalkI2cLinux::XWalkI2cLinux(cstring devicePath, uint32 retryCount)
    : XWalkI2cLinux(systemDeviceInterface(), devicePath, retryCount) {}

/**
 * @brief Constructs a Linux backend using an injected device boundary.
 *
 * @param[in,out] deviceInterface
 * Device-operation implementation that must outlive this backend.
 *
 * @param[in] devicePath
 * Non-null logical device path passed to `deviceInterface`.
 *
 * @param[in] retryCount
 * Number of permitted transaction attempts.
 */
XWalkI2cLinux::XWalkI2cLinux(XWalkI2cDevice &deviceInterface,
                             cstring devicePath, uint32 retryCount)
    : deviceInterfaceValue(deviceInterface),
      retryCountValue(validateRetryCount(retryCount)),
      fileDescriptor(openDevice(devicePath)) {
  XWALK_HAL_TRACE_UID1(RPI .028,
                       "Linux I2C backend constructed with %u retry attempts",
                       retryCountValue);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the backend and closes its owned descriptor.
 *
 * @post
 * A descriptor that was non-negative is passed to `close` and stored as `-1`.
 *
 * @note
 * The return value from the Linux `close` operation is not inspected.
 */
XWalkI2cLinux::~XWalkI2cLinux() {
  if (fileDescriptor >= 0) {
    deviceInterfaceValue.closeDevice(fileDescriptor);
    fileDescriptor = -1;
  }
}

} /* namespace xwalk::hal */
