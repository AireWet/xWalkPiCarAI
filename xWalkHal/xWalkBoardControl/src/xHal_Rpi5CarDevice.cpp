/******************************************************************************
 * @file        xHal_Rpi5CarDevice.cpp
 * @brief       Implements Robot HAT device-tree discovery and parsing.
 *
 * @details
 * Enumerates candidate HAT nodes, selects the supported UUID, reads product
 * properties, validates hexadecimal metadata, and publishes one complete result.
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

#include <filesystem>
#include <fstream>

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
 * @brief Repeats device-tree discovery from the configured root.
 *
 * @post
 * Previous results are replaced atomically at the object level after every
 * required property has been read and validated.
 *
 * @throws filesystemerror
 * If the root or a candidate property cannot be inspected.
 *
 * @throws std::runtime_error
 * If a selected HAT property cannot be read or parsed.
 */
void XWalkDevice::refresh()
{
    const filesystempath hatPath = findSupportedHatPath();
    const hal::boolean hatPathEmpty =
        static_cast<hal::boolean>(
            hatPath.empty());
    if (hatPathEmpty)
    {
        resetInformation();
        return;
    }

    XWalkDeviceInformation detectedInformation{};
    detectedInformation.productName =
        readProperty(hatPath, XHAL_RPI5CAR_DEVICE_PRODUCT_PROPERTY, false);
    const string productIdText =
        readProperty(hatPath, XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY, true);
    detectedInformation.productId =
        parseHexProperty(productIdText, XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY);
    const string productVersionText =
        readProperty(hatPath, XHAL_RPI5CAR_DEVICE_PRODUCT_VERSION_PROPERTY, true);
    detectedInformation.productVersion =
        parseHexProperty(productVersionText, XHAL_RPI5CAR_DEVICE_PRODUCT_VERSION_PROPERTY);
    detectedInformation.uuid =
        readProperty(hatPath, XHAL_RPI5CAR_DEVICE_UUID_PROPERTY, true);
    detectedInformation.vendor =
        readProperty(hatPath, XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY, false);
    detectedInformation.speakerEnablePin = XHAL_RPI5CAR_DEVICE_V5_SPEAKER_ENABLE_PIN;
    detectedInformation.motorMode = XHAL_RPI5CAR_DEVICE_V5_MOTOR_MODE;
    detectedInformation.model = XWalkDeviceModel::RobotHatV5;
    detectedInformation.detected = true;
    informationValue = std::move(detectedInformation);
}

/**
 * @brief Returns the most recent complete discovery result.
 *
 * @return
 * Product metadata, board configuration, model, and detection state.
 */
const XWalkDeviceInformation& XWalkDevice::information() const noexcept
{
    return informationValue;
}

/**
 * @brief Returns the configured device-tree root.
 *
 * @return
 * Owned root path retained by this detector.
 */
const filesystempath& XWalkDevice::deviceTreeRoot() const noexcept
{
    return deviceTreeRootValue;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Finds the first candidate node containing a supported UUID.
 *
 * @return
 * Candidate node path, or an empty path when no supported HAT is present.
 *
 * @throws filesystemerror
 * If the device-tree root cannot be enumerated or inspected.
 *
 * @throws std::runtime_error
 * If a candidate UUID property cannot be read.
 */
filesystempath XWalkDevice::findSupportedHatPath() const
{
    const stringvector entryNames = listFilesystemEntryNames(deviceTreeRootValue);
    for (const string& entryName : entryNames)
    {
        const hal::boolean entryNameMatched =
            static_cast<hal::boolean>(
                entryName.find(XHAL_RPI5CAR_DEVICE_HAT_NODE_MARKER) == string::npos);
        if (entryNameMatched)
        {
            continue;
        }
        const filesystempath candidatePath = deviceTreeRootValue / entryName;
        const filesystempath uuidPath = candidatePath / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY;
        const hal::boolean uuidFileUnavailable =
            static_cast<hal::boolean>(
                !filesystemEntryExists(uuidPath) || !isRegularFile(uuidPath));
        if (uuidFileUnavailable)
        {
            continue;
        }
        const string uuid =
            readProperty(candidatePath, XHAL_RPI5CAR_DEVICE_UUID_PROPERTY, true);
        if (uuid == XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID)
        {
            return candidatePath;
        }
    }
    return {};
}

/**
 * @brief Reads one property from a selected device-tree node.
 *
 * @param[in] hatPath
 * Existing selected HAT node path.
 *
 * @param[in] propertyName
 * Non-empty property filename below `hatPath`.
 *
 * @param[in] removeTrailingNull
 * `true` to remove one terminal null byte when present.
 *
 * @return
 * Complete property contents after the requested compatibility trimming.
 *
 * @throws std::runtime_error
 * If the property is absent, not regular, or cannot be read.
 */
string XWalkDevice::readProperty(const filesystempath& hatPath,
    stringview propertyName, boolean removeTrailingNull)
{
    const filesystempath propertyPath = hatPath / string(propertyName);
    const hal::boolean propertyFileUnavailable =
        static_cast<hal::boolean>(
            !filesystemEntryExists(propertyPath) || !isRegularFile(propertyPath));
    if (propertyFileUnavailable)
    {
        XHAL_THROW_RUNTIME_ERROR("Device-tree property is absent or not a regular file");
    }
    string propertyValue = readFileContents(propertyPath);
    const hal::boolean trailingNullPresent =
        static_cast<hal::boolean>(
            removeTrailingNull && !propertyValue.empty() && (propertyValue.back() == '\0'));
    if (trailingNullPresent)
    {
        propertyValue.pop_back();
    }
    return propertyValue;
}

/**
 * @brief Parses one unsigned hexadecimal device-tree property.
 *
 * @param[in] text
 * One through eight hexadecimal digits, optionally prefixed with `0x`.
 *
 * @param[in] propertyName
 * Non-null property name used in validation messages.
 *
 * @return
 * Parsed unsigned 32-bit value.
 *
 * @throws std::runtime_error
 * If the text is empty, malformed, or exceeds 32 bits.
 */
uint32 XWalkDevice::parseHexProperty(stringview text, cstring propertyName)
{
    size firstDigit = 0U;
    const hal::boolean textXInvalid =
        static_cast<hal::boolean>(
            (text.size() >= 2U) && (text[0U] == '0') &&
        ((text[1U] == 'x') || (text[1U] == 'X')));
    if (textXInvalid)
    {
        firstDigit = 2U;
    }
    const size digitCount = text.size() - firstDigit;
    if ((digitCount == 0U) ||
        (digitCount > static_cast<size>(XHAL_RPI5CAR_DEVICE_MAXIMUM_HEX_DIGITS)))
    {
        XHAL_THROW_RUNTIME_ERROR_DETAIL(propertyName, " is not a 32-bit hexadecimal value");
    }

    uint32 parsedValue = 0U;
    for (size index = firstDigit; index < text.size(); ++index)
    {
        const char character = text[index];
        uint32 digitValue = 0U;
        if ((character >= '0') && (character <= '9'))
        {
            digitValue = static_cast<uint32>(character - '0');
        }
        else if ((character >= 'a') && (character <= 'f'))
        {
            digitValue = static_cast<uint32>(character - 'a') + 10U;
        }
        else if ((character >= 'A') && (character <= 'F'))
        {
            digitValue = static_cast<uint32>(character - 'A') + 10U;
        }
        else
        {
            XHAL_THROW_RUNTIME_ERROR_DETAIL(propertyName, " contains a non-hexadecimal digit");
        }
        const uint32 shiftedValue = parsedValue * 16U;
        parsedValue = shiftedValue + digitValue;
    }
    return parsedValue;
}

} /* namespace xwalk::hal */
