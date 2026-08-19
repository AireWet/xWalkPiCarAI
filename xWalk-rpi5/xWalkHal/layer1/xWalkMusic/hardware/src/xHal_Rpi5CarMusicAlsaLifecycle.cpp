/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsaLifecycle.cpp
 * @brief       Implements ALSA music-adapter lifecycle and callback binding.
 *
 * @details
 * Validates injected decoding, stores the shared audio dependency, stops both
 * workers deterministically, and publishes the complete music callback table.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic ALSA Adapter
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

#include "xHal_Rpi5CarMusicAlsa.h"

#include "xHal_Rpi5CarTrace.h"

#include <csignal>
#include <pthread.h>
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
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs an adapter using the built-in PCM WAVE decoder.
     *
     * @param[in,out] sharedAudioBackend
     * Caller-owned ALSA backend that must outlive this adapter and its consumers.
     */
    XWalkMusicAlsa::XWalkMusicAlsa(XWalkAudioAlsa& sharedAudioBackend)
        : XWalkMusicAlsa(sharedAudioBackend, nullptr, systemOperations())
    {
    }

    /**
     * @brief Constructs an adapter using an injected file decoder.
     *
     * @param[in,out] sharedAudioBackend
     * Caller-owned ALSA backend that must outlive this adapter and its consumers.
     *
     * @param[in,out] context
     * Nullable non-owning decoder context that must outlive this adapter.
     *
     * @param[in] backendOperations
     * Operation table containing one non-null decoder callback.
     *
     * @throws std::invalid_argument
     * If the decoder callback is null.
     */
    XWalkMusicAlsa::XWalkMusicAlsa(XWalkAudioAlsa& sharedAudioBackend,
                                   contextpointer context,
                                   const XWalkMusicAlsaOperations& backendOperations)
        : audioBackend(&sharedAudioBackend), decoderContext(context), operations(backendOperations)
    {
        if (operations.decodeAudio == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Music ALSA adapter requires a decoder callback");
        }
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Stops and joins both retained playback workers.
     *
     * @warning
     * ALSA operations invoked during worker shutdown must not throw.
     */
    XWalkMusicAlsa::~XWalkMusicAlsa()
    {
        stopSoundWorker();
        stopMusicWorker();
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Returns the complete callback table bound through this adapter's
     * address.
     *
     * @return
     * Ten non-null callbacks suitable for constructing `XWalkMusic` with this
     * object as context.
     */
    XWalkMusicCallbacks XWalkMusicAlsa::callbacks() const noexcept
    {
        return {&enableOutput,
                &playSound,
                &playSoundBackground,
                &playMusic,
                &setMusicVolume,
                &stopMusic,
                &pauseMusic,
                &resumeMusic,
                &getSoundLength,
                &playTone};
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Starts one worker while blocking process-cancellation signals in the new thread.
     *
     * @param[in] workerOperation
     * Non-null playback member function executed by the new worker.
     *
     * @return
     * Joinable worker whose inherited signal mask blocks SIGINT and SIGTERM.
     *
     * @throws std::runtime_error
     * If the calling thread's signal mask cannot be changed or restored.
     */
    threadhandle XWalkMusicAlsa::startWorker(void (XWalkMusicAlsa::*workerOperation)() noexcept)
    {
        sigset_t cancellationSignals;
        sigset_t previousSignals;
        static_cast<void>(::sigemptyset(&cancellationSignals));
        static_cast<void>(::sigaddset(&cancellationSignals, SIGINT));
        static_cast<void>(::sigaddset(&cancellationSignals, SIGTERM));
        const int blockResult = ::pthread_sigmask(SIG_BLOCK, &cancellationSignals, &previousSignals);
        if (blockResult != 0)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Music worker cancellation signals could not be blocked");
        }

        threadhandle worker;
        try
        {
            worker = threadhandle(workerOperation, this);
        }
        catch (...)
        {
            static_cast<void>(::pthread_sigmask(SIG_SETMASK, &previousSignals, nullptr));
            throw;
        }

        const int restoreResult = ::pthread_sigmask(SIG_SETMASK, &previousSignals, nullptr);
        if (restoreResult != 0)
        {
            {
                const mutexlock lock(stateMutex);
                soundStopRequested = true;
                musicStopRequested = true;
            }
            worker.join();
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Music worker caller signal mask could not be restored");
        }
        return worker;
    }

    /**
     * @brief Converts a callback context into its required adapter.
     *
     * @param[in,out] context
     * Non-null pointer supplied to `XWalkMusic` during construction.
     *
     * @return
     * Adapter referenced by the callback context.
     *
     * @throws std::invalid_argument
     * If the callback context is null.
     */
    XWalkMusicAlsa& XWalkMusicAlsa::adapter(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Music ALSA callback context must not be null");
        }
        return *static_cast<XWalkMusicAlsa*>(context);
    }

    /**
     * @brief Requests and joins the retained background sound worker.
     *
     * @post
     * No background sound worker is joinable and its retained audio is empty.
     */
    void XWalkMusicAlsa::stopSoundWorker()
    {
        {
            const mutexlock lock(stateMutex);
            soundStopRequested = true;
        }
        const hal::boolean soundWorkerJoinable = static_cast<hal::boolean>(soundWorker.joinable());
        if (soundWorkerJoinable)
        {
            soundWorker.join();
        }
        const mutexlock lock(stateMutex);
        soundAudio = {};
        soundStopRequested = false;
    }

    /**
     * @brief Requests and joins the retained streamed-music worker.
     *
     * @post
     * No streamed-music worker is joinable and its retained state is reset.
     */
    void XWalkMusicAlsa::stopMusicWorker()
    {
        {
            const mutexlock lock(stateMutex);
            musicStopRequested = true;
            musicPauseRequested = false;
        }
        const hal::boolean musicWorkerJoinable = static_cast<hal::boolean>(musicWorker.joinable());
        if (musicWorkerJoinable)
        {
            musicWorker.join();
        }
        const mutexlock lock(stateMutex);
        musicAudio = {};
        musicStartFrame = 0U;
        musicLoopCount = 0;
        musicStopRequested = false;
        musicPauseRequested = false;
    }

} /* namespace xwalk::hal */
