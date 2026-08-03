/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordExampleLinux.h
 * @brief       Declares Linux Vosk composition for synchronous wake detection.
 *
 * @details
 * Adapts bounded ALSA capture, offline Vosk recognition, and console reporting
 * to the host-testable synchronous wake-word example contract.
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

#ifndef XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_LINUX_H

#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSttVoskWakeWordExample.h"

namespace xwalk::hal::example
{

/** @brief Composes synchronous wake recognition with Linux speech services. */
class XWalkSttVoskWakeWordExampleLinux final
{
private:

    /** @brief Temporarily bound speech coordinator valid only during `run()`. */
    XWalkSpeechToText* speechToTextObject{nullptr};

protected:

    /** @brief Resolves a callback context with one live speech binding. */
    static XWalkSttVoskWakeWordExampleLinux& adapter(contextpointer context);
    /** @brief Captures and recognizes one bounded utterance synchronously. */
    static string listen(contextpointer context, uint32 timeoutMs);
    /** @brief Prints one source-compatible status line. */
    static void report(contextpointer context, stringview message);

public:

    /**
     * @brief Waits synchronously for the configured wake phrase.
     * @param[in] maximumAttempts Recognition attempts from one through 1,200.
     * @param[in] timeoutMs Maximum capture duration for each attempt.
     * @param[in] microphoneDevice Non-empty ALSA capture device name.
     * @param[in] libraryName Non-empty Vosk shared-library name or path.
     * @param[in] modelPath Non-empty English Vosk model directory.
     * @warning Captures live microphone audio synchronously.
     */
    void run(uint32 maximumAttempts, uint32 timeoutMs,
        stringview microphoneDevice, stringview libraryName, stringview modelPath);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_LINUX_H */
