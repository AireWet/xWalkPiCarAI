/******************************************************************************
 * @file        xControllerApplicationSupport.h
 * @brief       Declares Raspberry Pi Controller application callbacks.
 *
 * @details
 * Defines the process contexts and synchronous callback adapters used while
 * the Raspberry Pi boot graph executes one Controller command.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XCONTROLLER_APPLICATION_SUPPORT_H
#define XCONTROLLER_APPLICATION_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xWalkControllerConfigTypes.h"

/******************************************************************************
 * Forward declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components used by the Controller application.
 */
namespace xwalk::hal
{
class XWalkMusic;
} /* namespace xwalk::hal */

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller application support for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @struct XWalkControllerBootContext
 * @brief Carries validated process arguments into one backend boot attempt.
 */
struct XWalkControllerBootContext
{
    /**
     * @brief Non-owning command-argument pointer that remains valid until boot returns.
     */
    const ::ctrl::stringvector* commandArguments{nullptr};

    /**
     * @brief Absolute packaged-data directory selected before boot.
     */
    ::ctrl::string resourceDirectory{};
};

/**
 * @struct XWalkControllerApplicationContext
 * @brief Carries audio and resource-path state through Controller callbacks.
 */
struct XWalkControllerApplicationContext
{
    /**
     * @brief Nullable non-owning Music pointer supplied by the selected boot graph.
     */
    hal::XWalkMusic* music{nullptr};

    /**
     * @brief Absolute packaged-data directory valid throughout command execution.
     */
    ::ctrl::string resourceDirectory{};
};

/******************************************************************************
 * Function declarations
 ******************************************************************************/

/** @brief Restores the application operation request before signal handlers are installed. */
void XWALK_resetOperationRequest() noexcept;

/**
 * @brief Disables normal traces and applies ordered trace requests before boot.
 * @param[in] applicationArguments Validated process-global trace requests.
 * @return `true` when the default and every requested update succeed.
 */
::ctrl::boolean xWalkApplyTraceConfiguration(
    const XWalkControllerApplicationArguments& applicationArguments);

/**
 * @brief Writes one CLI line to standard output.
 * @param[in] context Optional context; unused.
 * @param[in] line Text written synchronously followed by a newline.
 */
void XWALK_outputLine(::ctrl::contextpointer context, ::ctrl::stringview line);

/**
 * @brief Writes one prompt and reads one line from standard input.
 * @param[in] context Optional context; unused.
 * @param[in] prompt Prompt text written without a newline.
 * @return Owned response line, or `skip` when input reaches end-of-file.
 */
::ctrl::string XWALK_inputLine(::ctrl::contextpointer context,
    ::ctrl::stringview prompt);

/**
 * @brief Suspends the CLI on the calling thread.
 * @param[in] context Optional context; unused.
 * @param[in] durationMs Requested duration in milliseconds.
 */
void XWALK_delayMilliseconds(::ctrl::contextpointer context,
    ::ctrl::uint32 durationMs);

/**
 * @brief Requests graceful shutdown of the active operation from a process signal.
 * @param[in] signalNumber Delivered signal number; ignored after dispatch.
 */
void XWALK_requestOperationStop(int signalNumber) noexcept;

/**
 * @brief Reports whether the active operation may perform another bounded step.
 * @param[in] context Optional context; unused.
 * @return `true` until SIGINT or SIGTERM requests shutdown.
 */
::ctrl::boolean XWALK_continueOperation(::ctrl::contextpointer context) noexcept;

/**
 * @brief Executes one CLI audio operation through a caller-owned Music object.
 * @param[in,out] context Non-null application context that remains valid during the call.
 * @param[in] request Validated sound action, file path, and optional volume.
 * @return `true` after the Music backend accepts and completes the operation.
 */
::ctrl::boolean XWALK_performSound(::ctrl::contextpointer context,
    const XWalkSoundRequest& request);

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_APPLICATION_SUPPORT_H */
