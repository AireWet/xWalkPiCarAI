/******************************************************************************
 * @file        xHal_Rpi5CarUtilsLinuxCallbacks.cpp
 * @brief       Defines callback bridges for the Linux utility backend.
 *
 * @details
 * Builds callback tables and forwards opaque callback contexts to a live
 * `XWalkUtilsLinux` object without transferring ownership.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils Linux Backend
 *
 * @author      Joxy John
 * @date        2026-08-01
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

#include "xHal_Rpi5CarUtilsLinux.h"

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
 * @brief Returns the complete callback table for `XWalkUtils`.
 *
 * @return
 * Callback table whose context must point to this live backend.
 */
XWalkUtilsCallbacks XWalkUtilsLinux::utilityCallbacks() const noexcept
{
    return {&outputCallback, &volumeCallback, &commandCallback,
        &executableCallback, &ipCallback, &usernameCallback};
}

/**
 * @brief Returns the Linux standard-error redirect callback.
 *
 * @return
 * Callback whose context must point to this live backend.
 */
utilityredirectcallback XWalkUtilsLinux::stderrRedirectCallback() const noexcept
{
    return &redirectCallback;
}

/**
 * @brief Returns the Linux standard-error restore callback.
 *
 * @return
 * Non-throwing callback whose context must point to this live backend.
 */
utilityrestorecallback XWalkUtilsLinux::stderrRestoreCallback() const noexcept
{
    return &restoreCallback;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Bridges a utility-output callback to its backend object.
 *
 * @param[in,out] context
 * Non-null pointer to a live `XWalkUtilsLinux` object.
 *
 * @param[in] color
 * Supported terminal color.
 *
 * @param[in] message
 * Message view valid for this synchronous call.
 *
 * @param[in] ending
 * Record-ending view valid for this synchronous call.
 *
 * @param[in] flush
 * Flush request; Linux descriptor writes complete synchronously.
 */
void XWalkUtilsLinux::outputCallback(contextpointer context, XWalkUtilityColor color,
    stringview message, stringview ending, boolean flush)
{
    static_cast<XWalkUtilsLinux*>(context)->writeOutput(color, message, ending, flush);
}

/**
 * @brief Bridges a volume callback to its backend object.
 *
 * @param[in,out] context
 * Non-null pointer to a live `XWalkUtilsLinux` object.
 *
 * @param[in] volumePercent
 * Volume in the inclusive range zero through one hundred percent.
 */
void XWalkUtilsLinux::volumeCallback(contextpointer context, uint8 volumePercent)
{
    static_cast<XWalkUtilsLinux*>(context)->setSystemVolume(volumePercent);
}

/**
 * @brief Bridges a command callback to its backend object.
 *
 * @param[in,out] context
 * Non-null pointer to a live `XWalkUtilsLinux` object.
 *
 * @param[in] command
 * Trusted shell command text valid for this synchronous call.
 *
 * @param[in] user
 * Optional Linux user name.
 *
 * @param[in] group
 * Optional Linux group name.
 *
 * @return
 * Process exit status and combined output.
 */
XWalkCommandResult XWalkUtilsLinux::commandCallback(contextpointer context,
    stringview command, stringview user, stringview group)
{
    return static_cast<XWalkUtilsLinux*>(context)->runCommandProcess(command, user, group);
}

/**
 * @brief Bridges an executable lookup to its backend object.
 *
 * @param[in,out] context
 * Non-null pointer to a live `XWalkUtilsLinux` object.
 *
 * @param[in] executable
 * Executable name or path valid for this synchronous call.
 *
 * @return
 * `true` when an executable file is accessible; otherwise `false`.
 */
boolean XWalkUtilsLinux::executableCallback(contextpointer context, stringview executable)
{
    return static_cast<XWalkUtilsLinux*>(context)->executableExists(executable);
}

/**
 * @brief Bridges an IPv4 lookup to its backend object.
 *
 * @param[in,out] context
 * Non-null pointer to a live `XWalkUtilsLinux` object.
 *
 * @param[in] interfaceName
 * Linux network-interface name valid for this synchronous call.
 *
 * @return
 * Owned dotted-decimal IPv4 text, or an empty string when unavailable.
 */
string XWalkUtilsLinux::ipCallback(contextpointer context, stringview interfaceName)
{
    return static_cast<XWalkUtilsLinux*>(context)->interfaceIpv4(interfaceName);
}

/**
 * @brief Bridges a username lookup to its backend object.
 *
 * @param[in,out] context
 * Non-null pointer to a live `XWalkUtilsLinux` object.
 *
 * @return
 * Owned effective application username, or an empty string when unavailable.
 */
string XWalkUtilsLinux::usernameCallback(contextpointer context)
{
    return static_cast<XWalkUtilsLinux*>(context)->effectiveUsername();
}

/**
 * @brief Bridges standard-error suppression to its backend object.
 *
 * @param[in,out] context
 * Non-null pointer to a live `XWalkUtilsLinux` object.
 *
 * @return
 * Owned duplicate of the prior standard-error descriptor.
 */
int32 XWalkUtilsLinux::redirectCallback(contextpointer context)
{
    return static_cast<XWalkUtilsLinux*>(context)->redirectStandardError();
}

/**
 * @brief Bridges standard-error restoration to its backend object.
 *
 * @param[in,out] context
 * Non-null pointer to a live `XWalkUtilsLinux` object.
 *
 * @param[in] restoreToken
 * Descriptor returned by the paired redirect callback.
 */
void XWalkUtilsLinux::restoreCallback(contextpointer context, int32 restoreToken) noexcept
{
    static_cast<XWalkUtilsLinux*>(context)->restoreStandardError(restoreToken);
}

} /* namespace xwalk::hal */
