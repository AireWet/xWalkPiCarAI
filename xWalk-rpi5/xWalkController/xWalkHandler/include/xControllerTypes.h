/******************************************************************************
 * @file        xControllerTypes.h
 * @brief       Declares PiCar-X CLI callback and option types.
 *
 * @details
 * Defines the injected console, timing, cancellation, and audio boundary used by the command-line interface.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-07-31
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XCONTROLLER_TYPES_H
#define XCONTROLLER_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xWalkControllerConfigTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Parsed named CLI options stored as owned key-value text. */
using controlleroptions = ::ctrl::orderedmap<::ctrl::string, ::ctrl::string>;

/**
 * @brief Writes one complete CLI output line.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @param[in] line Output text that the callback must not retain.
 */
using controlleroutputcallback = void (*)(::ctrl::contextpointer context, ::ctrl::stringview line);

/**
 * @brief Reads one response after presenting a prompt.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @param[in] prompt Prompt text that the callback must not retain.
 * @return Owned response text without an implied line terminator.
 */
using controllerinputcallback = ::ctrl::string (*)(::ctrl::contextpointer context,
    ::ctrl::stringview prompt);

/**
 * @brief Suspends command execution for a bounded interval.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @param[in] durationMs Requested delay in milliseconds.
 */
using controllerdelaycallback = void (*)(::ctrl::contextpointer context, ::ctrl::uint32 durationMs);

/**
 * @brief Reports whether the active command may perform another bounded step.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @return `true` to continue; otherwise `false` to request emergency actuator shutdown.
 */
using controllercontinuecallback = ::ctrl::boolean (*)(::ctrl::contextpointer context);

/**
 * @brief Dispatches one optional platform audio operation.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @param[in] request Validated sound action, file path, and optional volume.
 * @return `true` when the platform accepted the request; otherwise `false`.
 */
using controllersoundcallback = ::ctrl::boolean (*)(::ctrl::contextpointer context,
    const XWalkSoundRequest& request);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Contains the complete application-owned CLI backend. */
struct XWalkControllerCallbacks
{
    /** @brief Non-null synchronous output operation. */
    controlleroutputcallback output{nullptr};
    /** @brief Non-null synchronous input operation. */
    controllerinputcallback input{nullptr};
    /** @brief Non-null delay operation. */
    controllerdelaycallback delay{nullptr};
    /** @brief Non-null cancellation query shared by every bounded command. */
    controllercontinuecallback continueOperation{nullptr};
    /** @brief Non-null audio operation that may report backend unavailability. */
    controllersoundcallback sound{nullptr};
};

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_TYPES_H */
