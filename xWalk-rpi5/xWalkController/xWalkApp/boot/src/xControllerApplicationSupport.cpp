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
#include "xHal_Rpi5CarTrace.h"

#include <iostream>
#include <pthread.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains process cancellation state private to this translation unit.
 */
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

    /**
     * @brief Applies ordered persistent trace settings before backend construction.
     * @param[in] applicationArguments Validated trace selectors and JSON paths.
     * @return `true` when every requested persistent update succeeds.
     */
    ::ctrl::boolean xWalkApplyTraceConfiguration(const XWalkControllerApplicationArguments& applicationArguments)
    {
        for (const ::ctrl::string& argument : applicationArguments.traceArguments)
        {
            const ::ctrl::boolean traceArgumentApplied = hal::XWalkTrace::applyGlobalTraceArgument(argument);
            if (traceArgumentApplied == false)
            {
                return false;
            }
        }
        return true;
    }

    /** @brief Restores the application operation request before signal handlers are
     * installed. */
    void XWALK_resetOperationRequest() noexcept
    {
        operationRequested = 1;
    }

    /**
     * @brief Installs cancellation handlers and blocks their signals during boot-worker construction.
     * @return `true` when both handlers and the temporary signal mask are established; otherwise `false`.
     * @post SIGINT and SIGTERM remain pending on the calling thread until operation handling is activated.
     */
    ::ctrl::boolean XWALK_prepareOperationSignalHandling() noexcept
    {
        struct sigaction cancellationAction
        {
        };
        cancellationAction.sa_handler = &XWALK_requestOperationStop;
        static_cast<void>(::sigemptyset(&cancellationAction.sa_mask));
        cancellationAction.sa_flags = 0;

        const ::ctrl::boolean interruptHandlerInstalled =
            static_cast<::ctrl::boolean>(::sigaction(SIGINT, &cancellationAction, nullptr) == 0);
        const ::ctrl::boolean terminationHandlerInstalled =
            static_cast<::ctrl::boolean>(::sigaction(SIGTERM, &cancellationAction, nullptr) == 0);
        sigset_t cancellationSignals;
        static_cast<void>(::sigemptyset(&cancellationSignals));
        static_cast<void>(::sigaddset(&cancellationSignals, SIGINT));
        static_cast<void>(::sigaddset(&cancellationSignals, SIGTERM));
        const ::ctrl::boolean cancellationSignalsBlocked =
            static_cast<::ctrl::boolean>(::pthread_sigmask(SIG_BLOCK, &cancellationSignals, nullptr) == 0);
        return static_cast<::ctrl::boolean>(interruptHandlerInstalled && terminationHandlerInstalled &&
                                            cancellationSignalsBlocked);
    }

    /**
     * @brief Unblocks cancellation signals on the Controller thread before command execution.
     * @return `true` when SIGINT and SIGTERM are unblocked; otherwise `false`.
     * @post Terminal input is interruptible and cancellation changes only the operation-request flag.
     */
    ::ctrl::boolean XWALK_activateOperationSignalHandling() noexcept
    {
        sigset_t cancellationSignals;
        static_cast<void>(::sigemptyset(&cancellationSignals));
        static_cast<void>(::sigaddset(&cancellationSignals, SIGINT));
        static_cast<void>(::sigaddset(&cancellationSignals, SIGTERM));
        return static_cast<::ctrl::boolean>(::pthread_sigmask(SIG_UNBLOCK, &cancellationSignals, nullptr) == 0);
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
        XWALK_CTRL_TRACE_UID0(CTRL .003, "Controller application wrote one output line");
    }

    /**
     * @brief Writes one prompt and reads one line from standard input.
     * @param[in] context Optional context; unused.
     * @param[in] prompt Prompt text written without a newline.
     * @return Owned response line, or `skip` when input reaches end-of-file.
     */
    ::ctrl::string XWALK_inputLine(::ctrl::contextpointer context, ::ctrl::stringview prompt)
    {
        static_cast<void>(context);
        std::cout << prompt << std::flush;
        ::ctrl::string response;
        const ::ctrl::boolean lineRead = static_cast<::ctrl::boolean>(std::getline(std::cin, response));
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
    void XWALK_delayMilliseconds(::ctrl::contextpointer context, ::ctrl::uint32 durationMs)
    {
        static_cast<void>(context);
        hal::common::sleepMilliseconds(durationMs);
    }

    /**
     * @brief Reads elapsed monotonic time for bounded Controller operations.
     * @param[in] context Optional context; unused.
     * @return Monotonic milliseconds from the platform steady-clock epoch.
     */
    ::ctrl::uint64 XWALK_monotonicMilliseconds(::ctrl::contextpointer context) noexcept
    {
        static_cast<void>(context);
        constexpr ::ctrl::uint64 microsecondsPerMillisecond{1'000U};
        const ::ctrl::uint64 elapsedMicroseconds = hal::common::monotonicMicroseconds();
        return elapsedMicroseconds / microsecondsPerMillisecond;
    }

    /**
     * @brief Requests graceful shutdown of the active operation from a process
     * signal.
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
     * @param[in,out] context Non-null application context that remains valid during
     * the call.
     * @param[in] request Validated sound action, file path, and optional volume.
     * @return `true` after the Music backend accepts and completes the operation.
     */
    ::ctrl::boolean XWALK_performSound(::ctrl::contextpointer context, const XWalkSoundRequest& request)
    {
        XWalkControllerApplicationContext& applicationContext =
            *static_cast<XWalkControllerApplicationContext*>(context);
        if (applicationContext.music == nullptr)
        {
            return false;
        }
        hal::XWalkMusic& music = *applicationContext.music;
        ::ctrl::string resolvedFilePath(request.filePath);
        if ((request.operation == XWalkSoundOperation::Play) || (request.operation == XWalkSoundOperation::Music))
        {
            const ::ctrl::filesystempath resolved = hal::resolveResourcePath(applicationContext.resourceDirectory,
                                                                             ::ctrl::filesystempath(request.filePath));
            resolvedFilePath = resolved.string();
            const ::ctrl::boolean readableRegularFileNotMatched =
                static_cast<::ctrl::boolean>(!hal::isReadableRegularFile(resolved));
            if (readableRegularFileNotMatched)
            {
                XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Unreadable sound resource: %s", resolvedFilePath.c_str());
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
                        static_cast<::ctrl::boolean>(!request.volumePercent.has_value());
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
