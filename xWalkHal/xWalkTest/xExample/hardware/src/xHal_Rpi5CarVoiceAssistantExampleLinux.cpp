/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantExampleLinux.cpp
 * @brief       Implements Linux composition for the voice-assistant example.
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

#include "xHal_Rpi5CarVoiceAssistantExampleLinux.h"

#include "xHal_Rpi5CarCameraLinux.h"
#include "xHal_Rpi5CarExampleConfig.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarSpeechRecognizerVosk.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"

#include <iostream>

namespace xwalk::hal::example
{

/** @brief Composes and runs every selected live provider. */
void XWalkVoiceAssistantExampleLinux::run(stringview apiKey,
    XWalkVoiceAssistantExampleInputMode inputMode, uint32 maximumRounds,
    uint32 timeoutMs, stringview microphoneDevice, stringview voskLibrary,
    stringview voskModelPath, stringview piperExecutable,
    stringview playbackExecutable, stringview cameraConnection,
    stringview captureExecutable, stringview cameraDevice)
{
    constexpr stringview endpoint{
        "https://api.openai.com/v1/chat/completions"};
    constexpr stringview modelName{"gpt-4o-mini"};
    constexpr stringview imagePath{XHAL_RPI5CAR_EXAMPLE_IMAGE_PATH};

    XWalkSpeechRecognizerVosk recognizer(voskLibrary, voskModelPath);
    XWalkSpeechToTextAlsa speechBackend(
        microphoneDevice, &recognizer, recognizer.operations());
    XWalkSpeechToText speechToText(&speechBackend, speechBackend.callbacks());
    const XWalkCameraConnection connection =
        XWalkCamera::connectionFromString(cameraConnection);
    XWalkCameraLinux cameraBackend(
        connection, captureExecutable, cameraDevice);
    const XWalkCameraConfiguration cameraConfiguration{640U, 480U, 5'000U};
    XWalkCamera camera(
        &cameraBackend, cameraBackend.callback(), cameraConfiguration);
    XWalkLanguageModelHttp modelBackend(
        XWalkLanguageModelHttpDialect::OpenAiChatCompletions,
        endpoint, modelName, apiKey);
    XWalkLanguageModel languageModel(&modelBackend, modelBackend.callbacks());
    XWalkTtsPiperExampleLinux piper(
        piperExecutable, playbackExecutable);

    speechToTextObject = &speechToText;
    languageModelObject = &languageModel;
    cameraObject = &camera;
    piperObject = &piper;
    const XWalkVoiceAssistantExampleCallbacks exampleCallbacks{
        &configure, &readKeyboard, &listen, &capture,
        &prompt, &speak, &report};
    XWalkVoiceAssistantExample example(this, exampleCallbacks);
    try
    {
        example.run(inputMode, maximumRounds, timeoutMs, imagePath);
        speechToTextObject = nullptr;
        languageModelObject = nullptr;
        cameraObject = nullptr;
        piperObject = nullptr;
    }
    catch (...)
    {
        speechToTextObject = nullptr;
        languageModelObject = nullptr;
        cameraObject = nullptr;
        piperObject = nullptr;
        throw;
    }
}

/** @brief Resolves one callback context with every live provider binding. */
XWalkVoiceAssistantExampleLinux& XWalkVoiceAssistantExampleLinux::adapter(
    contextpointer context)
{
    if (context == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-assistant Linux context must not be null");
    }
    XWalkVoiceAssistantExampleLinux& self =
        *static_cast<XWalkVoiceAssistantExampleLinux*>(context);
    if ((self.speechToTextObject == nullptr) ||
        (self.languageModelObject == nullptr) || (self.cameraObject == nullptr) ||
        (self.piperObject == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-assistant Linux providers are not bound");
    }
    return self;
}

/** @brief Applies exact instructions after validating every provider setting. */
void XWalkVoiceAssistantExampleLinux::configure(contextpointer context,
    const XWalkVoiceAssistantExampleConfiguration& configuration)
{
    XWalkVoiceAssistantExampleLinux& self = adapter(context);
    if ((configuration.name != "Buddy") ||
        (configuration.ttsModel != "en_US-ryan-low") ||
        (configuration.languageModel != "gpt-4o-mini") ||
        (configuration.speechLanguage != "en-us") ||
        !configuration.withImage || !configuration.keyboardEnabled ||
        !configuration.wakeEnabled)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-assistant source configuration is invalid");
    }
    self.languageModelObject->setInstructions(configuration.instructions);
}

/** @brief Prints one prompt and reads a terminal line. */
boolean XWalkVoiceAssistantExampleLinux::readKeyboard(
    contextpointer context, string& inputText)
{
    static_cast<void>(adapter(context));
    std::cout << ">>> " << std::flush;
    return static_cast<boolean>(std::getline(std::cin, inputText));
}

/** @brief Captures and recognizes one bounded utterance. */
string XWalkVoiceAssistantExampleLinux::listen(contextpointer context,
    uint32 timeoutMs, stringview language)
{
    XWalkVoiceAssistantExampleLinux& self = adapter(context);
    if (language != "en-us")
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-assistant speech language must be en-us");
    }
    return self.speechToTextObject->listen(timeoutMs);
}

/** @brief Captures one camera image and reports success. */
boolean XWalkVoiceAssistantExampleLinux::capture(
    contextpointer context, stringview imagePath)
{
    return !adapter(context).cameraObject->capture(imagePath).empty();
}

/** @brief Sends one exact-model text and image prompt. */
string XWalkVoiceAssistantExampleLinux::prompt(contextpointer context,
    stringview model, stringview text, stringview imagePath)
{
    XWalkVoiceAssistantExampleLinux& self = adapter(context);
    if (model != "gpt-4o-mini")
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-assistant language model must be gpt-4o-mini");
    }
    return self.languageModelObject->prompt(text, imagePath);
}

/** @brief Synthesizes one exact-model Piper response. */
void XWalkVoiceAssistantExampleLinux::speak(contextpointer context,
    stringview model, stringview text)
{
    adapter(context).piperObject->speakText(model, text);
}

/** @brief Prints one source-visible welcome or response. */
void XWalkVoiceAssistantExampleLinux::report(
    contextpointer context, stringview text)
{
    static_cast<void>(adapter(context));
    std::cout << text << '\n';
}

} /* namespace xwalk::hal::example */
