/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWithoutStreamExampleLinux.cpp
 * @brief       Implements Linux composition for non-streaming Vosk recognition.
 *
 * @details
 * Composes the project ALSA capture and Vosk provider and prints each exact
 * prompt or unlabeled final transcript produced by the core coordinator.
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

#include "xHal_Rpi5CarSttVoskWithoutStreamExampleLinux.h"

#include "xHal_Rpi5CarSpeechRecognizerVosk.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"

#include "xHal_Rpi5CarTrace.h"
#include <iostream>

namespace xwalk::hal::example
{

    /** @brief Composes live dependencies and runs final-result sessions. */
    void XWalkSttVoskWithoutStreamExampleLinux::run(uint32 sessionCount,
                                                    uint32 timeoutMs,
                                                    stringview microphoneDevice,
                                                    stringview libraryName,
                                                    stringview modelPath)
    {
        XWalkSpeechRecognizerVosk recognizer(libraryName, modelPath);
        XWalkSpeechToTextAlsa alsa(microphoneDevice, &recognizer, recognizer.operations());
        XWalkSpeechToText speechToText(&alsa, alsa.callbacks());
        speechToTextObject = &speechToText;
        const XWalkSttVoskWithoutStreamExampleCallbacks exampleCallbacks{&listen, &report};
        XWalkSttVoskWithoutStreamExample example(this, exampleCallbacks);
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
    XWalkSttVoskWithoutStreamExampleLinux& XWalkSttVoskWithoutStreamExampleLinux::adapter(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Non-streaming Vosk Linux context must not be null");
        }
        XWalkSttVoskWithoutStreamExampleLinux& self = *static_cast<XWalkSttVoskWithoutStreamExampleLinux*>(context);
        if (self.speechToTextObject == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Non-streaming Vosk Linux adapter has no speech binding");
        }
        return self;
    }

    /** @brief Captures and recognizes one bounded utterance. */
    string XWalkSttVoskWithoutStreamExampleLinux::listen(contextpointer context, uint32 timeoutMs)
    {
        return adapter(context).speechToTextObject->listen(timeoutMs);
    }

    /** @brief Prints one literal prompt or final transcript. */
    void XWalkSttVoskWithoutStreamExampleLinux::report(contextpointer context, stringview text)
    {
        static_cast<void>(adapter(context));
        std::cout << text << '\n';
    }

} /* namespace xwalk::hal::example */
