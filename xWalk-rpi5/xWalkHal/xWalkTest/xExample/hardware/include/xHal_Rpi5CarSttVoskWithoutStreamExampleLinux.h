/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWithoutStreamExampleLinux.h
 * @brief       Declares Linux composition for non-streaming Vosk recognition.
 *
 * @details
 * Adapts bounded ALSA capture, offline final-result recognition, and console
 * output to the host-testable non-streaming example contract.
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

#ifndef XHAL_RPI5CAR_STT_VOSK_WITHOUT_STREAM_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_STT_VOSK_WITHOUT_STREAM_EXAMPLE_LINUX_H

#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSttVoskWithoutStreamExample.h"

namespace xwalk::hal::example
{

    /** @brief Composes synchronous final-result recognition with Linux services. */
    class XWalkSttVoskWithoutStreamExampleLinux final
    {
        private:
            /** @brief Temporarily bound speech coordinator valid only during `run()`. */
            XWalkSpeechToText* speechToTextObject{nullptr};

        protected:
            /** @brief Resolves a callback context with one live speech binding. */
            static XWalkSttVoskWithoutStreamExampleLinux& adapter(contextpointer context);
            /** @brief Captures and recognizes one bounded utterance. */
            static string listen(contextpointer context, uint32 timeoutMs);
            /** @brief Prints one literal prompt or final transcript. */
            static void report(contextpointer context, stringview text);

        public:
            /**
             * @brief Runs bounded non-streaming Vosk sessions.
             * @param[in] sessionCount Session count from one through 100.
             * @param[in] timeoutMs Maximum capture duration for each session.
             * @param[in] microphoneDevice Non-empty ALSA capture device name.
             * @param[in] libraryName Non-empty Vosk shared-library name or path.
             * @param[in] modelPath Non-empty English Vosk model directory.
             * @warning Captures live microphone audio synchronously.
             */
            void run(uint32 sessionCount,
                     uint32 timeoutMs,
                     stringview microphoneDevice,
                     stringview libraryName,
                     stringview modelPath);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_STT_VOSK_WITHOUT_STREAM_EXAMPLE_LINUX_H */
