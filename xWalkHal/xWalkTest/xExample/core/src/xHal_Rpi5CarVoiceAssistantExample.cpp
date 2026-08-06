/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantExample.cpp
 * @brief       Implements the bounded multimodal voice-assistant example.
 *
 * @project     xWalk Firmware
 * @module      xExample
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarVoiceAssistantExample.h"

namespace xwalk::hal::example
{

/** @brief Binds and validates the complete operation table. */
XWalkVoiceAssistantExample::XWalkVoiceAssistantExample(contextpointer context,
    const XWalkVoiceAssistantExampleCallbacks& exampleCallbacks):
    callbackContext(context), callbacks(exampleCallbacks)
{
    if ((callbacks.configure == nullptr) ||
        (callbacks.readKeyboard == nullptr) || (callbacks.listen == nullptr) ||
        (callbacks.capture == nullptr) || (callbacks.prompt == nullptr) ||
        (callbacks.speak == nullptr) || (callbacks.report == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-assistant example requires a complete callback table");
    }
}

/** @brief Acquires one prompt from keyboard or a wake-gated microphone. */
boolean XWalkVoiceAssistantExample::acquirePrompt(
    XWalkVoiceAssistantExampleInputMode inputMode, uint32 timeoutMs,
    string& promptText)
{
    if (inputMode == XWalkVoiceAssistantExampleInputMode::Keyboard)
    {
        return callbacks.readKeyboard(callbackContext, promptText);
    }
    if (inputMode != XWalkVoiceAssistantExampleInputMode::WakeWord)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-assistant input mode is unsupported");
    }

    const string wakeText = callbacks.listen(
        callbackContext, timeoutMs, configurationValue.speechLanguage);
    if (wakeText != configurationValue.wakeWord)
    {
        return false;
    }
    const hal::boolean answerOnWakeAvailable =
        static_cast<hal::boolean>(
            !configurationValue.answerOnWake.empty());
    if (answerOnWakeAvailable)
    {
        callbacks.speak(callbackContext, configurationValue.ttsModel,
            configurationValue.answerOnWake);
    }
    promptText = callbacks.listen(
        callbackContext, timeoutMs, configurationValue.speechLanguage);
    return !promptText.empty();
}

/** @brief Runs bounded configured input, image, model, and speech rounds. */
void XWalkVoiceAssistantExample::run(
    XWalkVoiceAssistantExampleInputMode inputMode, uint32 maximumRounds,
    uint32 timeoutMs, stringview imagePath)
{
    if ((maximumRounds == 0U) ||
        (maximumRounds > XHAL_RPI5CAR_VOICE_ASSISTANT_EXAMPLE_MAXIMUM_ROUNDS) ||
        (timeoutMs == 0U) || (timeoutMs > 300'000U))
    {
        XHAL_THROW_OUT_OF_RANGE(
            "Voice-assistant round count or timeout is outside its range");
    }
    const hal::boolean configurationInvalid =
        static_cast<hal::boolean>(
            ((inputMode == XWalkVoiceAssistantExampleInputMode::Keyboard) &&
        !configurationValue.keyboardEnabled) ||
        ((inputMode == XWalkVoiceAssistantExampleInputMode::WakeWord) &&
        !configurationValue.wakeEnabled) ||
        (configurationValue.withImage && imagePath.empty()));
    if (configurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-assistant input mode or image path is invalid");
    }

    callbacks.configure(callbackContext, configurationValue);
    callbacks.report(callbackContext, configurationValue.welcome);
    callbacks.speak(callbackContext, configurationValue.ttsModel,
        configurationValue.welcome);
    for (uint32 roundIndex = 0U; roundIndex < maximumRounds; ++roundIndex)
    {
        string promptText;
        const hal::boolean promptAcquired =
            acquirePrompt(inputMode, timeoutMs, promptText);
        if (promptAcquired == false)
        {
            if (inputMode == XWalkVoiceAssistantExampleInputMode::Keyboard)
            {
                break;
            }
            continue;
        }
        stringview selectedImagePath{};
        if (configurationValue.withImage)
        {
            const hal::boolean imageCaptured =
                callbacks.capture(callbackContext, imagePath);
            if (imageCaptured == false)
            {
                XHAL_THROW_RUNTIME_ERROR(
                    "Voice-assistant image capture failed");
            }
            selectedImagePath = imagePath;
        }
        const string response = callbacks.prompt(callbackContext,
            configurationValue.languageModel, promptText, selectedImagePath);
        callbacks.report(callbackContext, response);
        const hal::boolean responseAvailable =
            static_cast<hal::boolean>(
                !response.empty());
        if (responseAvailable)
        {
            callbacks.speak(callbackContext, configurationValue.ttsModel,
                response);
        }
    }
}

} /* namespace xwalk::hal::example */
