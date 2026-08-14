/******************************************************************************
 * @file        xHal_Rpi5CarSpiLinuxLifecycle.cpp
 * @brief       Implements Linux SPI device configuration and lifecycle.
 *
 * @details
 * Opens one spidev descriptor, applies bounded mode, word-size, and clock
 * settings, and closes the descriptor during deterministic destruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Linux Backend
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

#include "xHal_Rpi5CarSpiDeviceLinux.h"
#include "xHal_Rpi5CarSpiLinux.h"

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

/** @brief Contains the process-lifetime production device interface. */
namespace {

/** @brief Returns the process-lifetime production SPI device interface. */
XWalkSpiDevice &systemDeviceInterface() {
  static XWalkSpiDeviceLinux deviceInterface;
  return deviceInterface;
}

} /* namespace */

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates settings and opens one Linux SPI device node.
 * @param[in] devicePath Non-null, non-empty device path.
 * @param[in] configuration Requested Linux SPI configuration.
 * @return Owned non-negative descriptor.
 * @throws std::invalid_argument If the path is empty or speed is zero.
 * @throws std::out_of_range If mode or word size is outside its range.
 * @throws std::runtime_error If the device cannot be opened.
 */
int32 XWalkSpiLinux::openValidatedDevice(
    cstring devicePath, const XWalkSpiConfiguration &configuration) {
  if ((devicePath == nullptr) || (devicePath[0U] == '\0')) {
    XWALK_HAL_ERROR(XWALK_INVAL, "SPI device path must not be empty");
  }
  if (configuration.speedHz == 0U) {
    XWALK_HAL_ERROR(XWALK_INVAL, "SPI speed must be greater than zero");
  }
  if (configuration.mode > XHAL_RPI5CAR_SPI_MAXIMUM_MODE) {
    XWALK_HAL_ERROR(XWALK_RANGE, "SPI mode must be between zero and three");
  }
  if ((configuration.bitsPerWord == 0U) ||
      (configuration.bitsPerWord > XHAL_RPI5CAR_SPI_MAXIMUM_BITS_PER_WORD)) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "SPI bits per word must be between one and thirty-two");
  }
  XWALK_HAL_TRACE_UID1(RPI .050, "Opening Linux SPI device: %s", devicePath);
  const int32 descriptor = deviceInterfaceValue.openDevice(devicePath);
  if (descriptor < 0) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Unable to open Linux SPI device");
  }
  XWALK_HAL_TRACE_UID1(RPI .051, "Linux SPI device opened with descriptor %d",
                       descriptor);
  return descriptor;
}

/**
 * @brief Validates and applies SPI mode, word size, and speed.
 * @param[in] configuration Requested Linux SPI configuration.
 * @throws std::runtime_error If Linux rejects a setting.
 */
void XWalkSpiLinux::configure(const XWalkSpiConfiguration &configuration) {
  uint8 mode = configuration.mode;
  uint8 bitsPerWord = configuration.bitsPerWord;
  uint32 speedHz = configuration.speedHz;
  boolean configurationSucceeded =
      deviceInterfaceValue.configureMode(fileDescriptor, mode);
  if (configurationSucceeded) {
    configurationSucceeded =
        deviceInterfaceValue.configureBitsPerWord(fileDescriptor, bitsPerWord);
  }
  if (configurationSucceeded) {
    configurationSucceeded =
        deviceInterfaceValue.configureSpeed(fileDescriptor, speedHz);
  }
  const boolean configurationFailed = configurationSucceeded == false;
  if (configurationFailed) {
    deviceInterfaceValue.closeDevice(fileDescriptor);
    fileDescriptor = -1;
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Unable to configure Linux SPI device");
  }
  modeValue = mode;
  bitsPerWordValue = bitsPerWord;
  speedHzValue = speedHz;
  XWALK_HAL_TRACE_UID3(
      RPI .052,
      "Linux SPI configured with mode %u, %u bits per word, and %u Hertz",
      static_cast<uint32>(modeValue), static_cast<uint32>(bitsPerWordValue),
      speedHzValue);
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Opens and configures one Linux SPI device.
 * @param[in] devicePath Non-null, non-empty deployment-selected path.
 * @param[in] configuration Clock, mode, and word settings copied by value.
 */
XWalkSpiLinux::XWalkSpiLinux(cstring devicePath,
                             const XWalkSpiConfiguration &configuration)
    : XWalkSpiLinux(systemDeviceInterface(), devicePath, configuration) {}

/**
 * @brief Constructs the backend with an injected device-operation interface.
 * @param[in,out] deviceInterface Device boundary that must outlive this
 * backend.
 * @param[in] devicePath Non-null, non-empty logical device path.
 * @param[in] configuration Clock, mode, and word settings copied by value.
 */
XWalkSpiLinux::XWalkSpiLinux(XWalkSpiDevice &deviceInterface,
                             cstring devicePath,
                             const XWalkSpiConfiguration &configuration)
    : deviceInterfaceValue(deviceInterface),
      fileDescriptor(openValidatedDevice(devicePath, configuration)) {
  configure(configuration);
  XWALK_HAL_TRACE_UID0(RPI .053, "Linux SPI backend construction completed");
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Closes the owned Linux SPI descriptor. */
XWalkSpiLinux::~XWalkSpiLinux() {
  if (fileDescriptor >= 0) {
    deviceInterfaceValue.closeDevice(fileDescriptor);
    fileDescriptor = -1;
  }
}

} /* namespace xwalk::hal */
