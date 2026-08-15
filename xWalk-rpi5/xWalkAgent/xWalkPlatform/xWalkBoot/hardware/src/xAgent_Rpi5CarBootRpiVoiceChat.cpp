/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVoiceChat.cpp
 * @brief       Composes the Raspberry Pi local voice chatbot.
 *
 * @details
 * Binds configured Vosk capture, Piper speech, local Ollama inference, and
 * retained conversation history for one synchronous callback.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xAgent_Rpi5CarLocalVoiceChatbot.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarSpeechRecognizerVosk.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTextToSpeechPiper.h"

namespace xwalk::agent
{

    /**
     * @brief Runs the configured local voice chatbot.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] boardControl Caller-owned board controller.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runVoiceChat(agent::contextpointer context,
                                            bootapplicationcallback callback,
                                            hal::XWalkConfigStore& config,
                                            hal::XWalkBoardControl& boardControl,
                                            XWalkPicarx& picarx)
    {
        hal::XWalkSpeechRecognizerVosk recognizer(
            config.get("voice_vosk_library", "/usr/lib/xwalk/libvosk.so"),
            config.get("voice_vosk_model", "/usr/share/xwalk/models/vosk/vosk-model-small-en-us-0.15"));
        hal::XWalkSpeechToTextAlsa speechToTextBackend(
            config.get("voice_capture_device", "default"), &recognizer, recognizer.operations());
        hal::XWalkSpeechToText speechToText(&speechToTextBackend, speechToTextBackend.callbacks());
        hal::XWalkTextToSpeechPiper piper(config.get("voice_piper_executable", "piper"),
                                          config.get("voice_piper_playback_executable", "aplay"),
                                          config.get("local_voice_chatbot_piper_model", "en_US-amy-low"));
        hal::XWalkTextToSpeech textToSpeech(boardControl, &piper, piper.callback());
        hal::XWalkLanguageModelOllama modelBackend(
            config.get("local_voice_chatbot_ollama_endpoint", "http://127.0.0.1:11434/api/chat"),
            config.get("local_voice_chatbot_ollama_model", "llama3.2:3b"));
        hal::XWalkLanguageModel languageModel(&modelBackend, modelBackend.callbacks());
        const agent::uint32 maximumMessages = parseUnsigned(config.get("local_voice_chatbot_maximum_messages", "20"),
                                                            "local_voice_chatbot_maximum_messages",
                                                            hal::common::UINT32_MAXIMUM);
        languageModel.setMaximumMessages(maximumMessages);
        const hal::XWalkVoiceAssistantConfiguration assistantConfiguration{
            XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_INSTRUCTIONS, XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME};
        hal::XWalkVoiceAssistant voiceAssistant(speechToText, languageModel, textToSpeech, assistantConfiguration);
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.voiceAssistant = &voiceAssistant;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
