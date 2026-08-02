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
 * @brief Reads the current Robot HAT firmware version.
 *
 * @return
 * Major, minor, and patch values from three consecutive register bytes.
 *
 * @throws std::runtime_error
 * If the backend lacks atomic register reads or returns another byte count.
 */
XWalkFirmwareVersion XWalkFirmwareInfo::read()
{
    const bytevector versionBytes = i2cPointer->readRegister(addressValue,
        XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_REGISTER,
        XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_BYTE_COUNT);
    if (versionBytes.size() != XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_BYTE_COUNT)
    {
        XHAL_THROW_RUNTIME_ERROR("Firmware version read returned an invalid length");
    }

    return {
        versionBytes[XHAL_RPI5CAR_FIRMWARE_INFO_MAJOR_INDEX],
        versionBytes[XHAL_RPI5CAR_FIRMWARE_INFO_MINOR_INDEX],
        versionBytes[XHAL_RPI5CAR_FIRMWARE_INFO_PATCH_INDEX]
    };
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
string XWalkFirmwareInfo::readText()
{
    const XWalkFirmwareVersion version = read();
    return formatVersion(version);
}

/**
 * @brief Returns the selected Robot HAT I2C address.
 *
 * @return
 * First responding supported address selected during construction.
 */
uint8 XWalkFirmwareInfo::address() const noexcept
{
    return addressValue;
}

/**
 * @brief Returns the ported Robot HAT Python library version.
 *
 * @return
 * Static non-owning text view containing `2.5.5`.
 */
stringview XWalkFirmwareInfo::libraryVersion() noexcept
{
    return XHAL_RPI5CAR_ROBOT_HAT_LIBRARY_VERSION;
}

} /* namespace xwalk::hal */
