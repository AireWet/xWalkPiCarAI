/******************************************************************************
 * @file        xControllerApplicationSupport.cpp
 * @brief       Implements Raspberry Pi Controller application callbacks.
 *
 * @details
 * Owns process cancellation state and adapts terminal, timing, and audio
 * operations to the callback contracts consumed by Controller and Agent code.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerApplicationSupport.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"
#include "xHal_Rpi5CarMusic.h"

#include <iostream>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains process cancellation state private to this translation unit. */
namespace
{

/******************************************************************************
 * Static global variables
 ******************************************************************************/

/**
 * @brief Indicates whether the foreground Controller operation may continue.
 *
 * @details
 * Reset by the controlling thread before signal handlers are installed and
 * cleared only by an asynchronous SIGINT or SIGTERM handler.
 */
volatile sig_atomic_t operationRequested = 1;

} /* namespace */

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller application support for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/** @brief Restores the application operation request before signal handlers are installed. */
void XWALK_resetOperationRequest() noexcept
{
    operationRequested = 1;
}

/**
 * @brief Writes one CLI line to standard output.
 * @param[in] context Optional context; unused.
 * @param[in] line Text written synchronously followed by a newline.
 */
void XWALK_outputLine(::ctrl::contextpointer context, ::ctrl::stringview line)
{
    static_cast<void>(context);
    std::cout << line << '\n';
}

/**
 * @brief Writes one prompt and reads one line from standard input.
 * @param[in] context Optional context; unused.
 * @param[in] prompt Prompt text written without a newline.
 * @return Owned response line, or `skip` when input reaches end-of-file.
 */
::ctrl::string XWALK_inputLine(::ctrl::contextpointer context,
    ::ctrl::stringview prompt)
{
    static_cast<void>(context);
    std::cout << prompt << std::flush;
    ::ctrl::string response;
    const ::ctrl::boolean lineRead =
        static_cast<::ctrl::boolean>(std::getline(std::cin, response));
    if (lineRead == false)
    {
        return "skip";
    }
    return response;
}

/**
 * @brief Suspends the CLI on the calling thread.
 * @param[in] context Optional context; unused.
 * @param[in] durationMs Requested duration in milliseconds.
 */
void XWALK_delayMilliseconds(::ctrl::contextpointer context,
    ::ctrl::uint32 durationMs)
{
    static_cast<void>(context);
    hal::common::sleepMilliseconds(durationMs);
}

/**
 * @brief Requests graceful shutdown of the active operation from a process signal.
 * @param[in] signalNumber Delivered signal number; ignored after dispatch.
 */
void XWALK_requestOperationStop(int signalNumber) noexcept
{
    static_cast<void>(signalNumber);
    operationRequested = 0;
}

/**
 * @brief Reports whether the active operation may perform another bounded step.
 * @param[in] context Optional context; unused.
 * @return `true` until SIGINT or SIGTERM requests shutdown.
 */
::ctrl::boolean XWALK_continueOperation(::ctrl::contextpointer context) noexcept
{
    static_cast<void>(context);
    return operationRequested != 0;
}

/**
 * @brief Executes one CLI audio operation through a caller-owned Music object.
 *
 * @details
 * Sound-effect and music-file operations are synchronous because the one-shot
 * CLI must retain its ALSA composition until playback completes.
 *
 * @param[in,out] context Non-null application context that remains valid during the call.
 * @param[in] request Validated sound action, file path, and optional volume.
 * @return `true` after the Music backend accepts and completes the operation.
 */
::ctrl::boolean XWALK_performSound(::ctrl::contextpointer context,
    const XWalkSoundRequest& request)
{
    XWalkControllerApplicationContext& applicationContext =
        *static_cast<XWalkControllerApplicationContext*>(context);
    if (applicationContext.music == nullptr)
    {
        return false;
    }
    hal::XWalkMusic& music = *applicationContext.music;
    ::ctrl::string resolvedFilePath(request.filePath);
    if ((request.operation == XWalkSoundOperation::Play) ||
        (request.operation == XWalkSoundOperation::Music))
    {
        const ::ctrl::filesystempath resolved = hal::resolveResourcePath(
            applicationContext.resourceDirectory,
            ::ctrl::filesystempath(request.filePath));
        resolvedFilePath = resolved.string();
        const ::ctrl::boolean readableRegularFileNotMatched =
            static_cast<::ctrl::boolean>(
                !hal::isReadableRegularFile(resolved));
        if (readableRegularFileNotMatched)
        {
            std::cerr << "Unreadable sound resource: " << resolvedFilePath << '\n';
            return false;
        }
    }
    switch (request.operation)
    {
        case XWalkSoundOperation::Play:
        case XWalkSoundOperation::Music:
            music.soundPlay(resolvedFilePath, request.volumePercent);
            break;
        case XWalkSoundOperation::Volume:
            {
                const ::ctrl::boolean volumePercentProvided =
                    static_cast<::ctrl::boolean>(
                        !request.volumePercent.has_value());
                    if (volumePercentProvided)
            {
                return false;
            }
            }
            music.musicSetVolume(*request.volumePercent);
            break;
        case XWalkSoundOperation::Stop:
            music.musicStop();
            break;
    }
    return true;
}

} /* namespace xwalk::ctrl */
