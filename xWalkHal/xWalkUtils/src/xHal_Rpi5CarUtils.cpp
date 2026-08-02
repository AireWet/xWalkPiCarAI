/******************************************************************************
 * @file        xHal_Rpi5CarUtils.cpp
 * @brief       Defines backend-neutral Robot HAT utility operations.
 *
 * @details
 * Dispatches output, volume, command, executable, network, and username
 * operations and implements deterministic range mapping.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils
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

#include "xHal_Rpi5CarUtils.h"

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
 * @brief Writes one colored record through the output backend.
 *
 * @param[in] message
 * Message forwarded synchronously without encoding changes.
 *
 * @param[in] color
 * Valid terminal color associated with the message.
 *
 * @param[in] ending
 * Record terminator forwarded synchronously.
 *
 * @param[in] flush
 * `true` to request immediate backend flushing; otherwise `false`.
 */
void XWalkUtils::printColor(stringview message, XWalkUtilityColor color,
    stringview ending, boolean flush) const
{
    callbacks.output(backendContextPointer, validateColor(color), message, ending, flush);
}

/**
 * @brief Writes a white informational record through the output backend.
 *
 * @param[in] message
 * Message forwarded synchronously.
 *
 * @param[in] ending
 * Record terminator forwarded synchronously.
 *
 * @param[in] flush
 * `true` to request immediate flushing; otherwise `false`.
 */
void XWalkUtils::info(stringview message, stringview ending, boolean flush) const
{
    printColor(message, XWalkUtilityColor::White, ending, flush);
}

/**
 * @brief Writes a gray diagnostic record through the output backend.
 *
 * @param[in] message
 * Message forwarded synchronously.
 *
 * @param[in] ending
 * Record terminator forwarded synchronously.
 *
 * @param[in] flush
 * `true` to request immediate flushing; otherwise `false`.
 */
void XWalkUtils::debug(stringview message, stringview ending, boolean flush) const
{
    printColor(message, XWalkUtilityColor::Gray, ending, flush);
}

/**
 * @brief Writes a yellow warning record through the output backend.
 *
 * @param[in] message
 * Message forwarded synchronously.
 *
 * @param[in] ending
 * Record terminator forwarded synchronously.
 *
 * @param[in] flush
 * `true` to request immediate flushing; otherwise `false`.
 */
void XWalkUtils::warning(stringview message, stringview ending, boolean flush) const
{
    printColor(message, XWalkUtilityColor::Yellow, ending, flush);
}

/**
 * @brief Writes a red error record through the output backend.
 *
 * @param[in] message
 * Message forwarded synchronously.
 *
 * @param[in] ending
 * Record terminator forwarded synchronously.
 *
 * @param[in] flush
 * `true` to request immediate flushing; otherwise `false`.
 */
void XWalkUtils::error(stringview message, stringview ending, boolean flush) const
{
    printColor(message, XWalkUtilityColor::Red, ending, flush);
}

/**
 * @brief Applies a clamped system-volume percentage.
 *
 * @param[in] volumePercent
 * Requested percentage; values outside zero through one hundred are clamped.
 */
void XWalkUtils::setVolume(int32 volumePercent) const
{
    int32 clampedVolumePercent = volumePercent;
    if (clampedVolumePercent < XHAL_RPI5CAR_UTILS_MINIMUM_VOLUME_PERCENT)
    {
        clampedVolumePercent = XHAL_RPI5CAR_UTILS_MINIMUM_VOLUME_PERCENT;
    }
    else if (clampedVolumePercent > XHAL_RPI5CAR_UTILS_MAXIMUM_VOLUME_PERCENT)
    {
        clampedVolumePercent = XHAL_RPI5CAR_UTILS_MAXIMUM_VOLUME_PERCENT;
    }
    else
    {
        /* The requested percentage already satisfies the backend contract. */
    }

    callbacks.setVolume(backendContextPointer, static_cast<uint8>(clampedVolumePercent));
}

/**
 * @brief Executes one application-approved command synchronously.
 *
 * @param[in] command
 * Command text forwarded without modification.
 *
 * @param[in] user
 * Optional backend-defined user name; empty means unspecified.
 *
 * @param[in] group
 * Optional backend-defined group name; empty means unspecified.
 *
 * @return
 * Backend process status and owned combined output.
 */
XWalkCommandResult XWalkUtils::runCommand(stringview command, stringview user,
    stringview group) const
{
    return callbacks.runCommand(backendContextPointer, command, user, group);
}

/**
 * @brief Reports whether one executable can be resolved by the backend.
 *
 * @param[in] command
 * Executable name forwarded synchronously.
 *
 * @return
 * `true` when the executable can be resolved; otherwise `false`.
 */
boolean XWalkUtils::commandExists(stringview command) const
{
    return callbacks.executableExists(backendContextPointer, command);
}

/**
 * @brief Reports whether one command is installed according to the backend.
 *
 * @param[in] command
 * Executable name forwarded synchronously.
 *
 * @return
 * `true` when the executable can be resolved; otherwise `false`.
 */
boolean XWalkUtils::isInstalled(stringview command) const
{
    return callbacks.executableExists(backendContextPointer, command);
}

/**
 * @brief Reports whether one executable can be resolved by the backend.
 *
 * @param[in] executable
 * Executable name forwarded synchronously.
 *
 * @return
 * `true` when the executable can be resolved; otherwise `false`.
 */
boolean XWalkUtils::checkExecutable(stringview executable) const
{
    return callbacks.executableExists(backendContextPointer, executable);
}

/**
 * @brief Returns the first IPv4 address found for ordered interfaces.
 *
 * @param[in] interfaces
 * Ordered interface names queried until one returns non-empty text.
 *
 * @return
 * First non-empty address, or an empty string when none is available.
 */
string XWalkUtils::ipAddress(const stringvector& interfaces) const
{
    for (const string& interfaceName : interfaces)
    {
        const string address = callbacks.ipAddress(backendContextPointer, interfaceName);
        if (!address.empty())
        {
            return address;
        }
    }
    return {};
}

/**
 * @brief Returns an IPv4 address for one interface.
 *
 * @param[in] interfaceName
 * Interface name forwarded synchronously.
 *
 * @return
 * IPv4 text, or an empty string when unavailable.
 */
string XWalkUtils::ipAddress(stringview interfaceName) const
{
    return callbacks.ipAddress(backendContextPointer, interfaceName);
}

/**
 * @brief Returns the first address for the default wireless and Ethernet interfaces.
 *
 * @return
 * Address for `wlan0`, then `eth0`, or an empty string when neither is available.
 */
string XWalkUtils::ipAddress() const
{
    const stringvector interfaces{"wlan0", "eth0"};
    return ipAddress(interfaces);
}

/**
 * @brief Returns the effective application username.
 *
 * @return
 * Backend-provided owned username, including empty when unavailable.
 */
string XWalkUtils::username() const
{
    return callbacks.username(backendContextPointer);
}

/**
 * @brief Maps a finite value linearly between two numeric ranges.
 *
 * @param[in] input
 * Finite value to map or extrapolate.
 *
 * @param[in] inputMinimum
 * Finite input-range origin.
 *
 * @param[in] inputMaximum
 * Finite input-range end, different from `inputMinimum`.
 *
 * @param[in] outputMinimum
 * Finite output-range origin.
 *
 * @param[in] outputMaximum
 * Finite output-range end.
 *
 * @return
 * Linearly mapped or extrapolated value.
 */
float64 XWalkUtils::mapping(float64 input, float64 inputMinimum,
    float64 inputMaximum, float64 outputMinimum, float64 outputMaximum)
{
    if ((!XHAL_IS_FINITE(input)) || (!XHAL_IS_FINITE(inputMinimum)) ||
        (!XHAL_IS_FINITE(inputMaximum)) || (!XHAL_IS_FINITE(outputMinimum)) ||
        (!XHAL_IS_FINITE(outputMaximum)) || (inputMinimum == inputMaximum))
    {
        XHAL_THROW_INVALID_ARGUMENT("Utility mapping requires finite values and a non-zero input range");
    }

    const float64 inputOffset = input - inputMinimum;
    const float64 inputRange = inputMaximum - inputMinimum;
    const float64 outputRange = outputMaximum - outputMinimum;
    const float64 normalizedInput = inputOffset / inputRange;
    const float64 scaledOutput = normalizedInput * outputRange;
    return scaledOutput + outputMinimum;
}

} /* namespace xwalk::hal */
