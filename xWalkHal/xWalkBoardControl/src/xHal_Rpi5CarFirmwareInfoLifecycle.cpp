/******************************************************************************
 * @file        xHal_Rpi5CarFirmwareInfoLifecycle.cpp
 * @brief       Implements firmware-information address selection and lifecycle.
 *
 * @details
 * Probes supported Robot HAT addresses in compatibility order and binds the
 * selected address to a caller-owned I2C interface without taking ownership.
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
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Selects the first responding supported Robot HAT address.
 *
 * @param[in,out] i2c
 * Caller-owned interface used to probe addresses `0x14` then `0x15`.
 *
 * @return
 * First responding supported seven-bit address.
 *
 * @throws std::runtime_error
 * If neither supported address responds.
 */
uint8 XWalkFirmwareInfo::selectAddress(XWalkI2c& i2c)
{
    const hal::boolean candidateAddressAvailable =
        static_cast<hal::boolean>(
            i2c.probe(XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1));
    if (candidateAddressAvailable)
    {
        return XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1;
    }
    const hal::boolean secondaryAddressAvailable =
        static_cast<hal::boolean>(
            i2c.probe(XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_2));
    if (secondaryAddressAvailable)
    {
        return XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_2;
    }
    XHAL_THROW_RUNTIME_ERROR("Firmware information device was not found");
}

/**
 * @brief Formats a typed firmware version as dotted decimal text.
 *
 * @param[in] version
 * Major, minor, and patch components to format.
 *
 * @return
 * Owned text in `major.minor.patch` form.
 */
string XWalkFirmwareInfo::formatVersion(const XWalkFirmwareVersion& version)
{
    const string majorText = common::uint32ToString(static_cast<uint32>(version.major));
    const string minorText = common::uint32ToString(static_cast<uint32>(version.minor));
    const string patchText = common::uint32ToString(static_cast<uint32>(version.patch));
    return majorText + "." + minorText + "." + patchText;
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a reader and selects one responding Robot HAT address.
 *
 * @param[in,out] i2c
 * I2C dependency that must outlive this object and support register reads.
 *
 * @post
 * `address()` identifies the first responding supported address.
 *
 * @throws std::runtime_error
 * If neither supported Robot HAT address responds.
 */
XWalkFirmwareInfo::XWalkFirmwareInfo(XWalkI2c& i2c):
    i2cPointer(&i2c), addressValue(selectAddress(i2c))
{
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the reader without releasing its non-owning I2C dependency.
 */
XWalkFirmwareInfo::~XWalkFirmwareInfo() = default;

} /* namespace xwalk::hal */
