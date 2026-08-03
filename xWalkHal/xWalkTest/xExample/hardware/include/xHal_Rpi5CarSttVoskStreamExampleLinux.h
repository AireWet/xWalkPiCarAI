/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskStreamExampleLinux.h
 * @brief       Declares Linux Vosk composition for the streaming speech example.
 *
 * @details
 * Defines bounded ALSA capture, offline recognition, and console adaptation.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_STT_VOSK_STREAM_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_STT_VOSK_STREAM_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSttVoskStreamExample.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::example
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/** @brief Composes bounded ALSA capture, offline Vosk, and console reporting. */
class XWalkSttVoskStreamExampleLinux final
{
private:

    /** @brief Temporarily bound speech coordinator valid only during `run()`. */
    XWalkSpeechToText* speechToTextObject{nullptr};

protected:

    /** @brief Resolves a callback context with a bound speech coordinator. */
    static XWalkSttVoskStreamExampleLinux& adapter(contextpointer context);
    /** @brief Captures one bounded utterance and reports its final Vosk result. */
    static void listen(contextpointer context, uint32 timeoutMs,
        sttvoskstreamresultcallback reportResult);
    /** @brief Prints the source-compatible speech prompt. */
    static void reportPrompt(contextpointer context);
    /** @brief Prints one partial or final speech result. */
    static void reportResult(contextpointer context, boolean done, stringview text);

public:

    /**
     * @brief Runs bounded offline speech-recognition sessions.
     * @param[in] sessionCount Session count from one through 100.
     * @param[in] timeoutMs Capture timeout per session in milliseconds.
     * @param[in] microphoneDevice Non-empty ALSA capture device name.
     * @param[in] libraryName Non-empty Vosk shared-library name or path.
     * @param[in] modelPath Non-empty English Vosk model directory.
     * @warning Captures live microphone audio for the requested intervals.
     */
    void run(uint32 sessionCount, uint32 timeoutMs, stringview microphoneDevice,
        stringview libraryName, stringview modelPath);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_STT_VOSK_STREAM_EXAMPLE_LINUX_H */
