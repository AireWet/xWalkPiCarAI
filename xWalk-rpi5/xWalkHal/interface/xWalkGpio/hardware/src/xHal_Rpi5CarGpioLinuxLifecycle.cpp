/******************************************************************************
 * @file        xHal_Rpi5CarGpioLinuxLifecycle.cpp
 * @brief       Implements Linux GPIO backend lifecycle behavior.
 *
 * @details
 * Opens the GPIO chip device, stops the optional event worker, and closes owned
 *descriptors.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Linux Backend
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

#include "xHal_Rpi5CarGpioDeviceLinux.h"
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarLinuxHeaders.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/** @brief Contains the process-lifetime production GPIO device interface. */
namespace {

/** @brief Returns the process-lifetime production GPIO device interface. */
XWalkGpioDevice &systemDeviceInterface() {
  static XWalkGpioDeviceLinux deviceInterface;
  return deviceInterface;
}

} /* namespace */

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Opens a Linux GPIO chip device.
 *
 * @param[in] devicePath
 * Non-null path to the GPIO character device.
 *
 * @param[in] expectedName
 * Optional exact kernel chip name verified when non-empty.
 *
 * @param[in] expectedLabel
 * Optional exact kernel chip label verified when non-empty.
 *
 * @param[in] minimumLineCount
 * Minimum number of lines required from the selected controller.
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
int32 XWalkGpioLinux::openDevice(cstring devicePath, stringview expectedName,
                                 stringview expectedLabel,
                                 uint32 minimumLineCount) {
  if ((devicePath == nullptr) || (devicePath[0U] == '\0')) {
    XWALK_HAL_ERROR(XWALK_INVAL, "GPIO device path must not be empty");
  }
  XWALK_HAL_TRACE_UID1(RPI .068, "Opening Linux GPIO device: %s", devicePath);
  const int32 descriptor = deviceInterfaceValue.openDevice(devicePath);
  if (descriptor < 0) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Unable to open Linux GPIO device");
  }
  gpiochip_info information{};
  const boolean chipInformationRead =
      deviceInterfaceValue.readChipInformation(descriptor, &information);
  const boolean chipInformationReadFailed = chipInformationRead == false;
  if (chipInformationReadFailed) {
    deviceInterfaceValue.closeDevice(descriptor);
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Unable to inspect Linux GPIO device");
  }
  const string chipName{information.name};
  const string chipLabel{information.label};
  const boolean nameExpected = expectedName.empty() == false;
  const boolean labelExpected = expectedLabel.empty() == false;
  const boolean nameMismatch = nameExpected && (chipName != expectedName);
  const boolean labelMismatch = labelExpected && (chipLabel != expectedLabel);
  const boolean lineCountInsufficient = information.lines < minimumLineCount;
  const boolean identityOrLineCountInvalid =
      nameMismatch || labelMismatch || lineCountInsufficient;
  if (identityOrLineCountInvalid) {
    deviceInterfaceValue.closeDevice(descriptor);
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Linux GPIO device identity or line count "
                                   "does not match configuration");
  }
  XWALK_HAL_TRACE_UID4(RPI .074,
                       "Linux GPIO device opened with descriptor %d, name %s, "
                       "label %s, and %u lines",
                       descriptor, chipName.c_str(), chipLabel.c_str(),
                       information.lines);
  return descriptor;
}

/**
 * @brief Closes the owned line descriptor when one is open.
 *
 * @post
 * `lineDescriptor` is `-1`.
 */
void XWalkGpioLinux::releaseLine() noexcept {
  if (lineDescriptor >= 0) {
    deviceInterfaceValue.closeDevice(lineDescriptor);
    lineDescriptor = -1;
  }
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a Linux GPIO backend and opens its chip device.
 *
 * @param[in] devicePath
 * Non-null path to the GPIO character device.
 *
 * @param[in] expectedName
 * Optional exact kernel chip name verified before line claims.
 *
 * @param[in] expectedLabel
 * Optional exact kernel chip label verified before line claims.
 *
 * @param[in] minimumLineCount
 * Minimum controller line count required by the application graph.
 *
 * @post
 * The backend owns an open chip descriptor but no GPIO line.
 */
XWalkGpioLinux::XWalkGpioLinux(cstring devicePath, stringview expectedName,
                               stringview expectedLabel,
                               uint32 minimumLineCount)
    : XWalkGpioLinux(systemDeviceInterface(), devicePath, expectedName,
                     expectedLabel, minimumLineCount) {}

/**
 * @brief Constructs the backend with an injected GPIO device boundary.
 * @param[in,out] deviceInterface Device boundary that must outlive this
 * backend.
 * @param[in] devicePath Non-null, non-empty logical device path.
 * @param[in] expectedName Optional exact logical chip name.
 * @param[in] expectedLabel Optional exact logical chip label.
 * @param[in] minimumLineCount Minimum required controller line count.
 */
XWalkGpioLinux::XWalkGpioLinux(XWalkGpioDevice &deviceInterface,
                               cstring devicePath, stringview expectedName,
                               stringview expectedLabel,
                               uint32 minimumLineCount)
    : deviceInterfaceValue(deviceInterface),
      chipDescriptor(openDevice(devicePath, expectedName, expectedLabel,
                                minimumLineCount)) {
  XWALK_HAL_TRACE_UID0(RPI .076, "Linux GPIO backend construction completed");
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Stops event dispatch and closes all owned Linux descriptors.
 *
 * @post
 * The event worker is joined and both descriptors are closed.
 */
XWalkGpioLinux::~XWalkGpioLinux() {
  stopRequested.store(true);
  const boolean eventThreadJoinable = eventThread.joinable();
  if (eventThreadJoinable) {
    eventThread.join();
  }
  releaseLine();
  if (chipDescriptor >= 0) {
    deviceInterfaceValue.closeDevice(chipDescriptor);
    chipDescriptor = -1;
  }
}

} /* namespace xwalk::hal */
