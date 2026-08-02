/******************************************************************************
 * @file        xHal_Rpi5CarUtilsTypes.h
 * @brief       Declares utility results, colors, and platform callbacks.
 *
 * @details
 * Defines backend-neutral contracts for the terminal, process, volume,
 * network, user, clock, and standard-error services used by xWalk utilities.
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

#ifndef XHAL_RPI5CAR_UTILS_TYPES_H
#define XHAL_RPI5CAR_UTILS_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/** @brief Identifies the terminal colors exposed by the Python utilities. */
enum class XWalkUtilityColor : uint8
{
    Gray = 0U, /**< Gray diagnostic text corresponding to ANSI code `1;30`. */
    Red = 1U, /**< Red error text corresponding to ANSI code `0;31`. */
    Green = 2U, /**< Green success text corresponding to ANSI code `0;32`. */
    Yellow = 3U, /**< Yellow warning text corresponding to ANSI code `0;33`. */
    Blue = 4U, /**< Blue informational text corresponding to ANSI code `0;34`. */
    Purple = 5U, /**< Purple informational text corresponding to ANSI code `0;35`. */
    DarkGreen = 6U, /**< Python dark-green label corresponding to ANSI code `0;36`. */
    White = 7U /**< White normal informational text corresponding to ANSI code `0;37`. */
};

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Contains the status and combined output of one completed command. */
struct XWalkCommandResult
{
    int32 status{}; /**< Backend-defined process exit status; zero indicates success. */
    string output{}; /**< Owned combined standard-output and standard-error text. */
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Callback that writes one colored terminal record synchronously.
 *
 * @param[in,out] context
 * Nullable non-owning platform context.
 *
 * @param[in] color
 * Valid color associated with the record.
 *
 * @param[in] message
 * Message view valid only for this synchronous invocation.
 *
 * @param[in] ending
 * Record-ending view valid only for this synchronous invocation.
 *
 * @param[in] flush
 * `true` to request immediate flushing; otherwise `false`.
 */
using utilityoutputcallback = void (*)(contextpointer context, XWalkUtilityColor color,
    stringview message, stringview ending, boolean flush);

/**
 * @brief Callback that applies a clamped system-volume percentage.
 *
 * @param[in,out] context
 * Nullable non-owning platform context.
 *
 * @param[in] volumePercent
 * Volume in the inclusive range zero through one hundred percent.
 */
using utilityvolumecallback = void (*)(contextpointer context, uint8 volumePercent);

/**
 * @brief Callback that executes one application-approved command synchronously.
 *
 * @param[in,out] context
 * Nullable non-owning platform context.
 *
 * @param[in] command
 * Command text valid only for this synchronous invocation.
 *
 * @param[in] user
 * Optional backend-defined user; empty means unspecified.
 *
 * @param[in] group
 * Optional backend-defined group; empty means unspecified.
 *
 * @return
 * Process status and owned combined output.
 */
using utilitycommandcallback = XWalkCommandResult (*)(contextpointer context,
    stringview command, stringview user, stringview group);

/**
 * @brief Callback that checks whether one executable can be resolved.
 *
 * @param[in,out] context
 * Nullable non-owning platform context.
 *
 * @param[in] executable
 * Executable name valid only for this synchronous invocation.
 *
 * @return
 * `true` when the executable can be resolved; otherwise `false`.
 */
using utilityexecutablecallback = boolean (*)(contextpointer context, stringview executable);

/**
 * @brief Callback that returns an IPv4 address for one network interface.
 *
 * @param[in,out] context
 * Nullable non-owning platform context.
 *
 * @param[in] interfaceName
 * Interface name valid only for this synchronous invocation.
 *
 * @return
 * Owned IPv4 text, or an empty string when unavailable.
 */
using utilityipcallback = string (*)(contextpointer context, stringview interfaceName);

/**
 * @brief Callback that returns the effective application username.
 *
 * @param[in,out] context
 * Nullable non-owning platform context.
 *
 * @return
 * Owned username, or an empty string when unavailable.
 */
using utilityusernamecallback = string (*)(contextpointer context);

/**
 * @brief Callback that begins standard-error suppression and returns a restore token.
 *
 * @param[in,out] context
 * Nullable non-owning platform context.
 *
 * @return
 * Backend-defined token required to restore the prior standard-error state.
 */
using utilityredirectcallback = int32 (*)(contextpointer context);

/**
 * @brief Callback that restores standard error using a previous token.
 *
 * @param[in,out] context
 * Nullable non-owning platform context.
 *
 * @param[in] restoreToken
 * Token returned by the paired redirect callback.
 */
using utilityrestorecallback = void (*)(contextpointer context, int32 restoreToken);

/**
 * @brief Callback that returns non-decreasing monotonic time in microseconds.
 *
 * @param[in,out] context
 * Nullable non-owning platform context.
 *
 * @return
 * Non-decreasing time in microseconds from a backend-defined epoch.
 */
using utilityclockcallback = uint64 (*)(contextpointer context);

/**
 * @brief Callback that acquires one value for `XWalkLazyReader`.
 *
 * @tparam ValueType
 * Default-constructible value returned and cached by the reader.
 *
 * @param[in,out] context
 * Nullable non-owning acquisition context.
 *
 * @return
 * Newly acquired value owned by the caller after return.
 */
template<typename ValueType>
using utilityreadcallback = ValueType (*)(contextpointer context);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Groups the complete platform services required by `XWalkUtils`. */
struct XWalkUtilsCallbacks
{
    utilityoutputcallback output{nullptr}; /**< Writes one colored terminal record. */
    utilityvolumecallback setVolume{nullptr}; /**< Applies one volume percentage. */
    utilitycommandcallback runCommand{nullptr}; /**< Executes one approved command. */
    utilityexecutablecallback executableExists{nullptr}; /**< Resolves one executable. */
    utilityipcallback ipAddress{nullptr}; /**< Queries one network interface. */
    utilityusernamecallback username{nullptr}; /**< Queries the effective username. */
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_UTILS_TYPES_H */
