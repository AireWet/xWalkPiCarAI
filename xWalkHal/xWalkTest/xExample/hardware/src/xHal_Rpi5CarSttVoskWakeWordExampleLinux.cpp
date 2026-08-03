/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordExampleLinux.cpp
 * @brief       Implements Linux Vosk composition for synchronous wake detection.
 *
 * @details
 * Composes the project ALSA capture and Vosk provider and forwards final
 * transcripts to the bounded synchronous wake-word coordinator.
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

#include "xHal_Rpi5CarSttVoskWakeWordExampleLinux.h"

#include "xHal_Rpi5CarSpeechRecognizerVosk.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"

#include <iostream>

namespace xwalk::hal::example
{

/** @brief Composes live dependencies and waits for bounded wake detection. */
void XWalkSttVoskWakeWordExampleLinux::run(uint32 maximumAttempts,
    uint32 timeoutMs, stringview microphoneDevice, stringview libraryName,
    stringview modelPath)
{
    XWalkSpeechRecognizerVosk recognizer(libraryName, modelPath);
    XWalkSpeechToTextAlsa alsa(
        microphoneDevice, &recognizer, recognizer.operations());
    XWalkSpeechToText speechToText(&alsa, alsa.callbacks());
    speechToTextObject = &speechToText;
    const XWalkSttVoskWakeWordExampleCallbacks exampleCallbacks{&listen, &report};
    XWalkSttVoskWakeWordExample example(this, exampleCallbacks);
    try
    {
        example.run(maximumAttempts, timeoutMs);
        speechToTextObject = nullptr;
    }
    catch (...)
    {
        speechToTextObject = nullptr;
        throw;
    }
}

/** @brief Resolves one live Linux adapter and speech binding. */
XWalkSttVoskWakeWordExampleLinux& XWalkSttVoskWakeWordExampleLinux::adapter(
    contextpointer context)
{
    if (context == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Wake-word Linux context must not be null");
    }
    XWalkSttVoskWakeWordExampleLinux& self =
        *static_cast<XWalkSttVoskWakeWordExampleLinux*>(context);
    if (self.speechToTextObject == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Wake-word Linux adapter has no speech binding");
    }
    return self;
}

/** @brief Captures and recognizes one bounded utterance synchronously. */
string XWalkSttVoskWakeWordExampleLinux::listen(
    contextpointer context, uint32 timeoutMs)
{
    return adapter(context).speechToTextObject->listen(timeoutMs);
}

/** @brief Prints one source-compatible status line. */
void XWalkSttVoskWakeWordExampleLinux::report(
    contextpointer context, stringview message)
{
    static_cast<void>(adapter(context));
    std::cout << message << '\n';
}

} /* namespace xwalk::hal::example */
