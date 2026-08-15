/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordThreadExampleLinux.h
 * @brief       Declares Linux composition for threaded Vosk wake detection.
 *
 * @details
 * Owns one bounded recognition worker while adapting ALSA capture, offline
 * Vosk results, wake-state polling, timing, and console status reporting.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_THREAD_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_THREAD_EXAMPLE_LINUX_H

#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSttVoskWakeWordThreadExample.h"

namespace xwalk::hal::example
{

    /** @brief Owns the Linux recognition worker used by the wake-word example. */
    class XWalkSttVoskWakeWordThreadExampleLinux final
    {
        private:
            /** @brief Temporarily bound speech coordinator valid only during `run()`. */
            XWalkSpeechToText* speechToTextObject{nullptr};
            /** @brief Maximum duration of each worker recognition slice. */
            uint32 listenTimeoutMsValue{};
            /** @brief Set by the worker after recognizing the exact wake phrase. */
            atomicboolean wakeDetected{false};
            /** @brief Requests termination at the next recognition boundary. */
            atomicboolean stopRequested{false};
            /** @brief Reports that the recognition worker caught an exception. */
            atomicboolean workerFailed{false};
            /** @brief Owned listener worker joined before its dependencies leave scope. */
            threadhandle worker{};

        protected:
            /** @brief Resolves a callback context with a live speech binding. */
            static XWalkSttVoskWakeWordThreadExampleLinux& adapter(contextpointer context);
            /** @brief Starts one bounded background wake-word listener. */
            static void startListening(contextpointer context);
            /** @brief Reports wake detection or a worker failure boundary. */
            static boolean isWaked(contextpointer context);
            /** @brief Requests recognition cancellation and joins the listener. */
            static void stopListening(contextpointer context);
            /** @brief Waits for one source-compatible polling interval. */
            static void wait(contextpointer context, uint32 durationMilliseconds);
            /** @brief Prints one source-compatible status message. */
            static void report(contextpointer context, stringview message);
            /** @brief Runs bounded recognition slices until detection or cancellation. */
            void listenForWakeWord() noexcept;
            /** @brief Tests a transcript for the case-insensitive wake phrase. */
            static boolean containsWakeWord(stringview transcript);

        public:
            /** @brief Ensures that no listener worker remains joinable. */
            ~XWalkSttVoskWakeWordThreadExampleLinux() noexcept;

            XWalkSttVoskWakeWordThreadExampleLinux() = default;
            XWalkSttVoskWakeWordThreadExampleLinux(const XWalkSttVoskWakeWordThreadExampleLinux&) = delete;
            XWalkSttVoskWakeWordThreadExampleLinux(XWalkSttVoskWakeWordThreadExampleLinux&&) = delete;
            XWalkSttVoskWakeWordThreadExampleLinux& operator=(const XWalkSttVoskWakeWordThreadExampleLinux&) = delete;
            XWalkSttVoskWakeWordThreadExampleLinux& operator=(XWalkSttVoskWakeWordThreadExampleLinux&&) = delete;

            /**
             * @brief Runs bounded background wake-word detection cycles.
             * @param[in] detectionCount Required detections from one through 100.
             * @param[in] maximumPolls Maximum three-second polls per detection.
             * @param[in] listenTimeoutMs Maximum duration of each capture slice.
             * @param[in] microphoneDevice Non-empty ALSA capture device name.
             * @param[in] libraryName Non-empty Vosk shared-library name or path.
             * @param[in] modelPath Non-empty English Vosk model directory.
             * @warning Captures microphone audio on a background thread.
             */
            void run(uint32 detectionCount,
                     uint32 maximumPolls,
                     uint32 listenTimeoutMs,
                     stringview microphoneDevice,
                     stringview libraryName,
                     stringview modelPath);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_THREAD_EXAMPLE_LINUX_H */
