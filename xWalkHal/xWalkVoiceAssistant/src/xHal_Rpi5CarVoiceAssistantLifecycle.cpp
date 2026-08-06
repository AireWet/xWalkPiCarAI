/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantLifecycle.cpp
 * @brief       Defines xWalk voice-assistant lifecycle operations.
 *
 * @details
 * Binds caller-owned pipeline components, applies language-model instructions,
 * and controls deterministic assistant start and stop transitions.
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
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a coordinator from caller-owned pipeline components.
 *
 * @param[in,out] speechToText
 * Speech-recognition object that must outlive this coordinator.
 *
 * @param[in,out] languageModel
 * Language-model object that must outlive this coordinator.
 *
 * @param[in,out] textToSpeech
 * Speech-output object that must outlive this coordinator.
 *
 * @param[in] assistantConfiguration
 * Startup instructions and optional welcome text copied by value.
 *
 * @param[in,out] callbackContext
 * Nullable non-owning context used by optional callbacks.
 *
 * @param[in] assistantCallbacks
 * Optional lifecycle and response-parser callbacks copied by value.
 */
XWalkVoiceAssistant::XWalkVoiceAssistant(XWalkSpeechToText& speechToText,
    XWalkLanguageModel& languageModel, XWalkTextToSpeech& textToSpeech,
    const XWalkVoiceAssistantConfiguration& assistantConfiguration,
    contextpointer callbackContext, const XWalkVoiceAssistantCallbacks& assistantCallbacks)
    : speechToTextPointer(&speechToText),
      languageModelPointer(&languageModel),
      textToSpeechPointer(&textToSpeech),
      callbackContextPointer(callbackContext),
      callbacks(assistantCallbacks),
      configuration(assistantConfiguration),
      runningValue(false)
{
    languageModelPointer->setInstructions(configuration.instructions);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Stops a running assistant without releasing caller-owned dependencies. */
XWalkVoiceAssistant::~XWalkVoiceAssistant()
{
    if (runningValue)
    {
        stop();
    }
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/** @brief Starts the assistant and optionally speaks its welcome text. */
void XWalkVoiceAssistant::start()
{
    if (!runningValue)
    {
        runningValue = true;
        invokeEvent(callbacks.onStart);
        const hal::boolean welcomeAvailable =
            static_cast<hal::boolean>(
                !configuration.welcome.empty());
        if (welcomeAvailable)
        {
            textToSpeechPointer->speak(configuration.welcome);
        }
    }
}

/** @brief Stops recognition and reports the assistant stop event. */
void XWalkVoiceAssistant::stop()
{
    if (runningValue)
    {
        runningValue = false;
        speechToTextPointer->stop();
        invokeEvent(callbacks.onStop);
    }
}

/** @brief Reports whether the assistant has been started and not stopped. */
boolean XWalkVoiceAssistant::isRunning() const
{
    return runningValue;
}

} /* namespace xwalk::hal */
