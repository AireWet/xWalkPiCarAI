/******************************************************************************
 * @file        xHal_Rpi5CarDeviceLifecycle.cpp
 * @brief       Implements device-detector lifecycle and default state.
 *
 * @details
 * Binds an owned device-tree root, establishes Python-compatible legacy board
 * defaults, and performs the initial discovery scan during construction.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl
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

#include "xHal_Rpi5CarDevice.h"

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
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a device detector and immediately scans its root.
 *
 * @param[in] deviceTreeRoot
 * Non-empty device-tree root path; defaults to `/proc/device-tree`.
 *
 * @post
 * `information()` contains detected metadata or default board values.
 *
 * @throws std::invalid_argument
 * If the root path is empty.
 *
 * @throws filesystemerror
 * If the root cannot be enumerated or inspected.
 *
 * @throws std::runtime_error
 * If a selected HAT property cannot be read or parsed.
 */
XWalkDevice::XWalkDevice(stringview deviceTreeRoot)
    : deviceTreeRootValue(string(deviceTreeRoot)) {
  const hal::boolean deviceTreeRootEmpty =
      static_cast<hal::boolean>(deviceTreeRoot.empty());
  if (deviceTreeRootEmpty) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Device-tree root must not be empty");
  }
  refresh();
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the detector without modifying its device-tree source.
 */
XWalkDevice::~XWalkDevice() = default;

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Resets metadata and board configuration to Python-compatible defaults.
 *
 * @post
 * Metadata is empty, detection is false, and Robot HAT v4 pin and motor values
 * apply.
 */
void XWalkDevice::resetInformation() noexcept { informationValue = {}; }

} /* namespace xwalk::hal */
