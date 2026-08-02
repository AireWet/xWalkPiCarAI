/******************************************************************************
 * @file        xAgent_Rpi5CarControllerTypes.h
 * @brief       Declares PiCar-X CLI callback and option types.
 *
 * @details
 * Defines the injected console, timing, cancellation, and audio boundary used by the command-line interface.
 *
 * @project     xWalk Firmware
 * @module      xWalkController
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

#ifndef XAGENT_RPI5CAR_CONTROLLER_TYPES_H
#define XAGENT_RPI5CAR_CONTROLLER_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/** @brief Selects one audio operation requested by the CLI. */
enum class XWalkSoundOperation : hal::uint8
{
    /** @brief Plays one blocking sound-effect file. */
    Play,
    /** @brief Changes the shared audio volume without requiring a file. */
    Volume,
    /** @brief Starts one streamed music file. */
    Music,
    /** @brief Stops current music playback. */
    Stop
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Parsed named CLI options stored as owned key-value text. */
using controlleroptions = hal::orderedmap<hal::string, hal::string>;

/**
 * @brief Writes one complete CLI output line.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @param[in] line Output text that the callback must not retain.
 */
using controlleroutputcallback = void (*)(hal::contextpointer context, hal::stringview line);

/**
 * @brief Reads one response after presenting a prompt.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @param[in] prompt Prompt text that the callback must not retain.
 * @return Owned response text without an implied line terminator.
 */
using controllerinputcallback = hal::string (*)(hal::contextpointer context,
    hal::stringview prompt);

/**
 * @brief Suspends command execution for a bounded interval.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @param[in] durationMs Requested delay in milliseconds.
 */
using controllerdelaycallback = void (*)(hal::contextpointer context, hal::uint32 durationMs);

/**
 * @brief Reports whether the active command may perform another bounded step.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @return `true` to continue; otherwise `false` to request emergency actuator shutdown.
 */
using controllercontinuecallback = hal::boolean (*)(hal::contextpointer context);

/**
 * @brief Dispatches one optional platform audio operation.
 * @param[in,out] context Non-owning application context that outlives the CLI.
 * @param[in] operation Requested sound action.
 * @param[in] filePath File path for play or music, or empty text otherwise.
 * @param[in] volumePercent Optional volume in the inclusive range zero through one hundred percent.
 * @return `true` when the platform accepted the request; otherwise `false`.
 */
using controllersoundcallback = hal::boolean (*)(hal::contextpointer context,
    XWalkSoundOperation operation, hal::stringview filePath, hal::optionalfloat64 volumePercent);

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

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_CONTROLLER_TYPES_H */
