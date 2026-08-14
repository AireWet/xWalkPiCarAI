/******************************************************************************
 * @file        xHal_Rpi5CarFirmwareInfo.cpp
 * @brief       Implements Robot HAT firmware-version acquisition.
 *
 * @details
 * Performs one atomic register read, validates its exact length, converts the
 * three version bytes, and exposes dotted-decimal and library-version text.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarFirmwareInfo.h"
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
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Reads the current Robot HAT firmware version.
 *
 * @return
 * Major, minor, and patch values from three consecutive register bytes.
 *
 * @throws std::runtime_error
 * If the backend lacks atomic register reads or returns another byte count.
 */
XWalkFirmwareVersion XWalkFirmwareInfo::read() {
  const bytevector versionBytes = i2cPointer->readRegister(
      addressValue, XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_REGISTER,
      XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_BYTE_COUNT);
  const hal::boolean versionBytesDifferent = static_cast<hal::boolean>(
      versionBytes.size() != XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_BYTE_COUNT);
  if (versionBytesDifferent) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Firmware version read returned an invalid length");
  }

  const XWalkFirmwareVersion version{
      versionBytes[XHAL_RPI5CAR_FIRMWARE_INFO_MAJOR_INDEX],
      versionBytes[XHAL_RPI5CAR_FIRMWARE_INFO_MINOR_INDEX],
      versionBytes[XHAL_RPI5CAR_FIRMWARE_INFO_PATCH_INDEX]};
  XWALK_HAL_TRACE_UID3(RPI .329, "Robot HAT firmware version read as %u.%u.%u",
                       static_cast<uint32>(version.major),
                       static_cast<uint32>(version.minor),
                       static_cast<uint32>(version.patch));
  return version;
}

/**
 * @brief Reads and formats the current Robot HAT firmware version.
 *
 * @return
 * Owned dotted-decimal text in `major.minor.patch` form.
 *
 * @throws std::runtime_error
 * If firmware acquisition fails.
 */
string XWalkFirmwareInfo::readText() {
  const XWalkFirmwareVersion version = read();
  return formatVersion(version);
}

/**
 * @brief Returns the selected Robot HAT I2C address.
 *
 * @return
 * First responding supported address selected during construction.
 */
uint8 XWalkFirmwareInfo::address() const noexcept { return addressValue; }

/**
 * @brief Returns the ported Robot HAT Python library version.
 *
 * @return
 * Static non-owning text view containing `2.5.5`.
 */
stringview XWalkFirmwareInfo::libraryVersion() noexcept {
  return XHAL_RPI5CAR_ROBOT_HAT_LIBRARY_VERSION;
}

} /* namespace xwalk::hal */
