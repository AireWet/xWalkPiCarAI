/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistant.cpp
 * @brief       Defines xWalk voice-assistant round processing.
 *
 * @details
 * Implements synchronous recognition, model prompting, response parsing,
 * speech output, and optional lifecycle notification callbacks.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarVoiceAssistant.h"

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
 * Public member function definitions
 ******************************************************************************/

/** @brief Acquires one final speech-recognition result. */
string XWalkVoiceAssistant::listen(uint32 timeoutMs)
{
    requireRunning();
    const hal::boolean readyNotMatched =
        static_cast<hal::boolean>(
            !speechToTextPointer->isReady());
    if (readyNotMatched)
    {
        XHAL_THROW_RUNTIME_ERROR("Speech-to-text backend is not ready");
    }

    invokeEvent(callbacks.beforeListen);
    const string recognizedText = speechToTextPointer->listen(timeoutMs);
    invokeText(callbacks.afterListen, recognizedText);
    return recognizedText;
}

/** @brief Generates one final language-model response. */
string XWalkVoiceAssistant::think(stringview text, stringview imagePath)
{
    requireRunning();
    invokeText(callbacks.beforeThink, text);
    const string response = languageModelPointer->prompt(text, imagePath);
    invokeText(callbacks.afterThink, response);
    return response;
}

/** @brief Synthesizes one non-empty response with lifecycle callbacks. */
void XWalkVoiceAssistant::say(stringview text)
{
    requireRunning();
    const hal::boolean textAvailable =
        static_cast<hal::boolean>(
            !text.empty());
    if (textAvailable)
    {
        invokeText(callbacks.beforeSay, text);
        textToSpeechPointer->speak(text);
        invokeText(callbacks.afterSay, text);
    }
}

/** @brief Processes caller-supplied text through model, parser, and speech output. */
string XWalkVoiceAssistant::processText(stringview text, stringview imagePath)
{
    requireRunning();
    const string response = think(text, imagePath);
    const string parsedResponse = parseResponse(response);
    say(parsedResponse);
    invokeEvent(callbacks.onRoundComplete);
    return parsedResponse;
}

/** @brief Executes one microphone-to-speech assistant round. */
string XWalkVoiceAssistant::runRound(uint32 timeoutMs, stringview imagePath)
{
    requireRunning();
    const string recognizedText = listen(timeoutMs);
    const hal::boolean recognizedTextEmpty =
        static_cast<hal::boolean>(
            recognizedText.empty());
    if (recognizedTextEmpty)
    {
        invokeEvent(callbacks.onRoundComplete);
        return {};
    }

    invokeText(callbacks.onHeard, recognizedText);
    return processText(recognizedText, imagePath);
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/** @brief Rejects an operation that requires a started assistant. */
void XWalkVoiceAssistant::requireRunning() const
{
    if (!runningValue)
    {
        XHAL_THROW_LOGIC_ERROR("Voice assistant must be started before this operation");
    }
}

/** @brief Invokes an optional event callback. */
void XWalkVoiceAssistant::invokeEvent(voiceassistanteventcallback callback) const
{
    if (callback != nullptr)
    {
        callback(callbackContextPointer);
    }
}

/** @brief Invokes an optional text callback. */
void XWalkVoiceAssistant::invokeText(voiceassistanttextcallback callback, stringview text) const
{
    if (callback != nullptr)
    {
        callback(callbackContextPointer, text);
    }
}

/** @brief Applies the optional response parser. */
string XWalkVoiceAssistant::parseResponse(stringview response) const
{
    if (callbacks.parseResponse == nullptr)
    {
        return string(response);
    }
    return callbacks.parseResponse(callbackContextPointer, response);
}

} /* namespace xwalk::hal */
