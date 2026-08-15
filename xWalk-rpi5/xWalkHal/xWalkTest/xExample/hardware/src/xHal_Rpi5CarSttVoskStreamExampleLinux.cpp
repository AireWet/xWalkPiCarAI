/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskStreamExampleLinux.cpp
 * @brief       Implements Linux Vosk composition for the streaming speech
 *example.
 *
 * @details
 * Composes the project ALSA capture and Vosk provider with console output.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskStreamExampleLinux.h"

#include "xHal_Rpi5CarSpeechRecognizerVosk.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"

#include "xHal_Rpi5CarTrace.h"
#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::example
{

    /** @brief Runs bounded ALSA capture and offline Vosk recognition. */
    void XWalkSttVoskStreamExampleLinux::run(uint32 sessionCount,
                                             uint32 timeoutMs,
                                             stringview microphoneDevice,
                                             stringview libraryName,
                                             stringview modelPath)
    {
        XWalkSpeechRecognizerVosk recognizer(libraryName, modelPath);
        XWalkSpeechToTextAlsa alsa(microphoneDevice, &recognizer, recognizer.operations());
        XWalkSpeechToText speechToText(&alsa, alsa.callbacks());
        speechToTextObject = &speechToText;
        const XWalkSttVoskStreamExampleCallbacks exampleCallbacks{&listen, &reportPrompt, &reportResult};
        XWalkSttVoskStreamExample example(this, exampleCallbacks);
        try
        {
            example.run(sessionCount, timeoutMs);
            speechToTextObject = nullptr;
        }
        catch (...)
        {
            speechToTextObject = nullptr;
            throw;
        }
    }

    /** @brief Resolves one live Linux adapter and speech binding. */
    XWalkSttVoskStreamExampleLinux& XWalkSttVoskStreamExampleLinux::adapter(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Streaming Vosk Linux context must not be null");
        }
        XWalkSttVoskStreamExampleLinux& self = *static_cast<XWalkSttVoskStreamExampleLinux*>(context);
        if (self.speechToTextObject == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Streaming Vosk Linux adapter has no speech binding");
        }
        return self;
    }

    /** @brief Captures one utterance and emits the final-result boundary. */
    void XWalkSttVoskStreamExampleLinux::listen(contextpointer context,
                                                uint32 timeoutMs,
                                                sttvoskstreamresultcallback resultCallback)
    {
        XWalkSttVoskStreamExampleLinux& self = adapter(context);
        const string result = self.speechToTextObject->listen(timeoutMs);
        resultCallback(context, true, result);
    }

    /** @brief Prints the upstream prompt before each microphone capture. */
    void XWalkSttVoskStreamExampleLinux::reportPrompt(contextpointer context)
    {
        static_cast<void>(adapter(context));
        std::cout << "Say something\n";
    }

    /** @brief Prints one source-compatible partial or final result. */
    void XWalkSttVoskStreamExampleLinux::reportResult(contextpointer context, boolean done, stringview text)
    {
        static_cast<void>(adapter(context));
        if (done)
        {
            std::cout << "final:   " << text << '\n';
        }
        else
        {
            std::cout << "partial: " << text << '\r' << std::flush;
        }
    }

} /* namespace xwalk::hal::example */
