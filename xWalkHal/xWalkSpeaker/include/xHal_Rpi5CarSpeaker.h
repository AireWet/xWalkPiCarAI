/******************************************************************************
 * @file        xHal_Rpi5CarSpeaker.h
 * @brief       Declares bounded asynchronous speaker playback control.
 *
 * @details
 * Manages caller-identified playback tasks, progress, pause, resume, stopping,
 * speaker power state, and injected file-decoder and stream operations.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker
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

#ifndef XHAL_RPI5CAR_SPEAKER_H
#define XHAL_RPI5CAR_SPEAKER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeakerTypes.h"

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
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkSpeaker
 * @brief Controls speaker output and bounded asynchronous audio-file tasks.
 *
 * @details
 * Uses caller-owned decoding and stream callbacks, retains decoded audio until
 * each worker is joined, and supports up to eight simultaneous playback tasks.
 */
class XWalkSpeaker
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Nullable non-owning context passed to every backend callback.
         *
         * @note
         * Any non-null backend object must outlive this controller and all tasks.
         */
        contextpointer backendContext{nullptr};
        /** @brief Complete backend callback table copied during construction. */
        XWalkSpeakerCallbacks callbacks{};
        /** @brief Mutex protecting speaker state and all playback-task metadata. */
        mutable mutexhandle stateMutex{};
        /** @brief Fixed task storage avoiding an unbounded playback-task container. */
        speakertaskslots taskSlots{};
        /** @brief `true` after successful output enable and before successful disable. */
        boolean speakerEnabled{};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Validates that every required backend callback is non-null.
         *
         * @param[in] backendCallbacks
         * Callback table to validate before any operation is invoked.
         *
         * @throws std::invalid_argument
         * If any callback is null.
         */
        static void validateCallbacks(const XWalkSpeakerCallbacks& backendCallbacks);

        /**
         * @brief Selects a Python-compatible decoder from a filename extension.
         *
         * @param[in] filePath
         * Non-empty audio-file path ending in a supported extension.
         *
         * @return
         * SoundFile for WAV, FLAC, or OGG; otherwise Librosa for MP3, M4A, AAC, or WMA.
         *
         * @throws std::invalid_argument
         * If the extension is absent or unsupported.
         */
        static XWalkSpeakerAudioHandler audioHandler(stringview filePath);

        /**
         * @brief Validates decoded sample rate, channels, and frame alignment.
         *
         * @param[in] audioData
         * Decoded interleaved audio returned by the backend.
         *
         * @throws std::runtime_error
         * If metadata is zero, samples do not form complete frames, or a sample is non-finite.
         */
        static void validateAudioData(const XWalkSpeakerAudioData& audioData);

        /**
         * @brief Executes one playback task.
         *
         * @param[in] taskIndex
         * Valid occupied task-slot index below the maximum task count.
         *
         * @post
         * The stream is closed and the slot is marked finished.
         */
        void playbackLoop(size taskIndex) noexcept;

        /**
         * @brief Finds an occupied unfinished task while the state mutex is held.
         *
         * @param[in] taskId
         * Non-empty playback identifier to locate.
         *
         * @return
         * Matching slot index or `XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX`.
         *
         * @pre
         * The calling execution context owns `stateMutex`.
         */
        size findTaskSlot(stringview taskId) const noexcept;

        /**
         * @brief Clears a joined task slot while the state mutex is held.
         *
         * @param[in] taskIndex
         * Valid slot index whose worker is not joinable.
         *
         * @pre
         * The calling execution context owns `stateMutex`.
         */
        void clearTaskSlot(size taskIndex) noexcept;

        /** @brief Joins and clears every worker that has already finished. */
        void reapFinishedTasks();

        /** @brief Requests, joins, and clears every retained playback task. */
        void stopAllTasks();

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a speaker controller and enables physical output.
         *
         * @param[in,out] context
         * Non-owning backend context; nullability is backend-specific.
         *
         * @param[in] backendCallbacks
         * Complete callback table copied into the controller.
         *
         * @pre
         * Any non-null context outlives this controller and all playback workers.
         *
         * @post
         * `isSpeakerEnabled()` returns `true` after the enable callback succeeds.
         *
         * @throws std::invalid_argument
         * If any required backend callback is null.
         */
        XWalkSpeaker(contextpointer context, const XWalkSpeakerCallbacks& backendCallbacks);

        /**
         * @brief Stops all tasks and disables output.
         *
         * @note
         * Backend stream and output resources are released through callbacks;
         * the non-owning context itself is never released.
         *
         * @warning
         * Backend cleanup callbacks invoked during destruction must not throw.
         */
        ~XWalkSpeaker();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction because workers retain object identity. */
        XWalkSpeaker(XWalkSpeaker&&) = delete;
        /** @brief Disables copying of task, mutex, and backend state. */
        XWalkSpeaker(const XWalkSpeaker&) = delete;
        /** @brief Disables move assignment because workers retain object identity. */
        XWalkSpeaker& operator=(XWalkSpeaker&&) = delete;
        /** @brief Disables copy assignment of task, mutex, and backend state. */
        XWalkSpeaker& operator=(const XWalkSpeaker&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /** @brief Enables physical output if it is currently disabled. */
        void enableSpeaker();

        /** @brief Disables physical output if it is currently enabled. */
        void disableSpeaker();

        /**
         * @brief Reports the logical speaker-enable state.
         *
         * @return
         * `true` after successful enable and before successful disable.
         */
        boolean isSpeakerEnabled() const noexcept;

        /**
         * @brief Decodes an audio file and starts background playback.
         *
         * @param[in] filePath
         * Existing regular WAV, FLAC, OGG, MP3, M4A, AAC, or WMA file.
         *
         * @return
         * Non-empty identifier unique among active playback tasks.
         *
         * @throws std::runtime_error
         * If the file is absent, not regular, decoded data is invalid, no task
         * slot is available, or the backend returns a duplicate identifier.
         *
         * @throws std::invalid_argument
         * If the path is empty or has an unsupported extension.
         *
         * @throws filesystemerror
         * If the filesystem cannot inspect the requested path.
         *
         * @throws std::system_error
         * If the playback worker cannot be created.
         */
        string play(stringview filePath);

        /**
         * @brief Returns progress for one active playback task.
         *
         * @param[in] taskId
         * Identifier returned by `play()`.
         *
         * @return
         * Frame position, ratios, durations, and active-writing state.
         *
         * @throws std::invalid_argument
         * If no active task has the identifier.
         *
         */
        XWalkSpeakerProgress getProgress(stringview taskId);

        /**
         * @brief Pauses one active playback task at its next chunk boundary.
         *
         * @param[in] taskId
         * Identifier returned by `play()`.
         *
         * @throws std::invalid_argument
         * If no active task has the identifier.
         *
         */
        void pause(stringview taskId);

        /**
         * @brief Resumes one paused playback task.
         *
         * @param[in] taskId
         * Identifier returned by `play()`.
         *
         * @throws std::invalid_argument
         * If no active task has the identifier.
         *
         */
        void resume(stringview taskId);

        /**
         * @brief Stops, joins, and removes one playback task.
         *
         * @param[in] taskId
         * Identifier returned by `play()`.
         *
         * @return
         * `true` when an active task is stopped; otherwise `false`.
         *
         */
        boolean stop(stringview taskId);

        /**
         * @brief Lists identifiers of every active playback task.
         *
         * @return
         * Active task identifiers in bounded slot order.
         *
         */
        stringvector listTasks();
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEAKER_H */
