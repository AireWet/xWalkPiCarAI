/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVoiceActiveMode.cpp
 * @brief       Composes shared Raspberry Pi voice-active vehicle services.
 *
 * @details
 * Loads speech, model, audio, status-LED, and credential-environment selections
 * before publishing one profile-specific service graph. Camera services are
 * composed only for image-enabled non-Jarvis profiles.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xAgent_Rpi5CarCameraCapture.h"
#include "xAgent_Rpi5CarGptCar.h"
#include "xAgent_Rpi5CarSelfDrive.h"
#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"
#include "xHal_Rpi5CarAudioAlsa.h"
#include "xHal_Rpi5CarCameraLinux.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarMusicAlsa.h"
#include "xHal_Rpi5CarMusicSndFileDecoder.h"
#include "xHal_Rpi5CarSpeechRecognizerVosk.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTextToSpeechAlsa.h"
#include "xHal_Rpi5CarTextToSpeechEspeak.h"
#include "xHal_Rpi5CarTextToSpeechPiper.h"
#include "xHal_Rpi5CarWebSearch.h"

#include "xHal_Rpi5CarTrace.h"
#include <cstdlib>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent
{

    /**
     * @brief Composes one configured voice-active vehicle profile.
     * @param[in] mode VoiceActiveCar, VoiceActiveCarGpt, or GptCar request macro.
     * @param[in] parameters Non-owning application, vehicle, configuration, and
     * GPIO dependencies valid through the synchronous dispatch.
     * @return Status returned by the configured application callback.
     * @throws std::runtime_error If the selected credential environment is empty.
     * @pre Every required pointer in `parameters` is non-null.
     */
    agent::int32 XWalkBootRpi::runVoiceActiveMode(agent::uint8 mode, const xAgentContext& parameters)
    {
        hal::XWalkConfigStore& config = *parameters.config;
        hal::XWalkBoardControl& boardControl = *parameters.board;
        XWalkPicarx& picarx = *parameters.picarx;
        const agent::string voskLibrary = config.get("voice_vosk_library", "/usr/lib/xwalk/libvosk.so");
        const agent::string voskModel =
            config.get("voice_vosk_model", "/usr/share/xwalk/models/vosk/vosk-model-small-en-us-0.15");
        const agent::string captureDevice = config.get("voice_capture_device", "default");
        const agent::string playbackDevice = config.get("voice_playback_device", "default");
        const agent::string mixerDevice = config.get("voice_mixer_device", "default");
        const agent::string mixerElement = config.get("voice_mixer_element", "PCM");
        const agent::string espeakExecutable = config.get("voice_espeak_executable", "espeak-ng");
        const agent::string espeakVoice = config.get("voice_espeak_voice", "en");
        const agent::string piperExecutable = config.get("voice_piper_executable", "piper");
        const agent::string piperPlaybackExecutable = config.get("voice_piper_playback_executable", "aplay");
        const agent::string piperModel =
            config.get("voice_active_car_gpt_piper_model", XWalkVoiceActiveCarGpt::SPEECH_VOICE);

        agent::string modelProvider = config.get("voice_language_model_provider", "ollama");
        agent::string modelEndpoint = config.get(
            "voice_language_model_endpoint", config.get("voice_ollama_endpoint", "http://127.0.0.1:11434/api/chat"));
        const agent::string modelEnvironment = config.get("voice_language_model_model_environment", "");
        const agent::boolean modelEnvironmentConfigured =
            static_cast<agent::boolean>(modelEnvironment.empty() == false);
        const agent::cstring configuredModel =
            modelEnvironmentConfigured ? std::getenv(modelEnvironment.c_str()) : nullptr;
        agent::string modelName = configuredModel == nullptr
                                      ? config.get("voice_language_model_model", config.get("voice_ollama_model", ""))
                                      : agent::string(configuredModel);
        const agent::string modelApiKeyEnvironment = config.get("voice_language_model_api_key_environment", "");
        const agent::boolean apiEnvironmentConfigured =
            static_cast<agent::boolean>(modelApiKeyEnvironment.empty() == false);
        const agent::cstring configuredApiKey =
            apiEnvironmentConfigured ? std::getenv(modelApiKeyEnvironment.c_str()) : nullptr;
        agent::string modelApiKey = configuredApiKey == nullptr ? agent::string{} : agent::string(configuredApiKey);
        agent::string maximumOutputTokensText = config.get("voice_language_model_maximum_output_tokens", "1024");
        agent::string modelTimeoutText = config.get("voice_language_model_timeout_ms", "120000");
        agent::string maximumMessagesText = "20";

        agent::string profileApiKeyEnvironment;
        const agent::boolean rollyProfile = static_cast<agent::boolean>(mode == XWALK_BOOT_VOICE_ACTIVE_CAR_REQ);
        const agent::boolean jarvisProfile = static_cast<agent::boolean>(mode == XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ);
        if (rollyProfile)
        {
            modelProvider = "openai";
            profileApiKeyEnvironment = config.get("voice_active_car_api_key_environment", "OPENAI_API_KEY");
            modelEndpoint = config.get("voice_active_car_endpoint", XWalkVoiceActiveCar::MODEL_ENDPOINT);
            modelName = config.get("voice_active_car_model", XWalkVoiceActiveCar::MODEL_NAME);
            maximumOutputTokensText = config.get("voice_active_car_maximum_output_tokens", "1024");
        }
        else if (jarvisProfile)
        {
            modelProvider = config.get("voice_active_car_gpt_provider", XWalkVoiceActiveCarGpt::MODEL_PROVIDER);
            profileApiKeyEnvironment =
                config.get("voice_active_car_gpt_api_key_environment", XWalkVoiceActiveCarGpt::API_KEY_ENVIRONMENT);
            modelEndpoint = config.get("voice_active_car_gpt_endpoint", XWalkVoiceActiveCarGpt::MODEL_ENDPOINT);
            modelName = config.get("voice_active_car_gpt_model", XWalkVoiceActiveCarGpt::MODEL_NAME);
            maximumOutputTokensText = config.get("voice_active_car_gpt_maximum_output_tokens",
                                                 std::to_string(XWalkVoiceActiveCarGpt::MAXIMUM_OUTPUT_TOKENS));
            modelTimeoutText =
                config.get("voice_active_car_gpt_timeout_ms", std::to_string(XWalkVoiceActiveCarGpt::MODEL_TIMEOUT_MS));
            maximumMessagesText = config.get("voice_active_car_gpt_maximum_messages",
                                             std::to_string(XWalkVoiceActiveCarGpt::MAXIMUM_MESSAGES));
        }
        else
        {
            modelProvider = "openai";
            profileApiKeyEnvironment = config.get("gpt_car_api_key_environment", "OPENAI_API_KEY");
            modelEndpoint = config.get("gpt_car_endpoint", XWalkGptCar::MODEL_ENDPOINT);
            modelName = config.get("gpt_car_model", XWalkGptCar::MODEL_NAME);
            maximumOutputTokensText = config.get("gpt_car_maximum_output_tokens", "1024");
        }
        const hal::XWalkLanguageModelHttpDialect modelDialect =
            hal::XWalkLanguageModelHttp::dialectFromString(modelProvider);
        const agent::boolean credentialRequired =
            static_cast<agent::boolean>(modelDialect == hal::XWalkLanguageModelHttpDialect::OpenAiChatCompletions);
        const agent::cstring profileApiKey =
            profileApiKeyEnvironment.empty() ? nullptr : std::getenv(profileApiKeyEnvironment.c_str());
        const agent::boolean profileApiKeyMissing = static_cast<agent::boolean>(
            credentialRequired && ((profileApiKey == nullptr) || (profileApiKey[0U] == '\0')));
        if (profileApiKeyMissing)
        {
            const std::string exceptionMessage =
                std::string(profileApiKeyEnvironment).append(" must be set for the selected voice-active mode");
            XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, exceptionMessage);
        }
        modelApiKey = profileApiKey == nullptr ? agent::string{} : agent::string(profileApiKey);

        const agent::uint32 maximumOutputTokens = parseUnsigned(maximumOutputTokensText,
                                                                "voice_language_model_maximum_output_tokens",
                                                                XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_MAXIMUM_OUTPUT_TOKENS);
        const agent::uint32 modelTimeoutMs = parseUnsigned(
            modelTimeoutText, "voice_language_model_timeout_ms", XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_TIMEOUT_MS);
        const agent::uint32 maximumMessages = parseUnsigned(maximumMessagesText,
                                                            "voice_active_car_gpt_maximum_messages",
                                                            XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_MESSAGES);

        hal::XWalkSpeechEndpointConfiguration endpointConfiguration{};
        endpointConfiguration.minimumSpeechMilliseconds = parseSeconds(
            config.get("voice_vosk_endpoint_start_seconds", "0.5"), "voice_vosk_endpoint_start_seconds", 30'000U);
        endpointConfiguration.trailingSilenceMilliseconds = parseSeconds(
            config.get("voice_vosk_endpoint_end_seconds", "1.0"), "voice_vosk_endpoint_end_seconds", 30'000U);
        endpointConfiguration.maximumUtteranceMilliseconds = parseSeconds(
            config.get("voice_vosk_endpoint_max_seconds", "15.0"), "voice_vosk_endpoint_max_seconds", 30'000U);
        endpointConfiguration.silencePeakThreshold = parseUnsigned(
            config.get("voice_vosk_silence_peak_threshold", "500"), "voice_vosk_silence_peak_threshold", 32'767U);
        endpointConfiguration.traceTranscript =
            parseBoolean(config.get("voice_vosk_trace_transcript", "false"), "voice_vosk_trace_transcript");
        hal::XWalkSpeechRecognizerVosk recognizer(voskLibrary, voskModel);
        hal::XWalkSpeechToTextAlsa speechToTextBackend(
            captureDevice, &recognizer, recognizer.operations(), endpointConfiguration);
        hal::XWalkSpeechToText speechToText(&speechToTextBackend, speechToTextBackend.callbacks());
        hal::XWalkAudioAlsa audioBackend(playbackDevice, mixerDevice, mixerElement);
        hal::XWalkTextToSpeechEspeak espeak(espeakExecutable, espeakVoice);
        hal::XWalkTextToSpeechAlsa textToSpeechBackend(audioBackend, &espeak, espeak.operations());
        hal::XWalkTextToSpeechPiper piper(piperExecutable, piperPlaybackExecutable, piperModel);
        agent::contextpointer textToSpeechContext = &textToSpeechBackend;
        hal::texttospeechspeakcallback textToSpeechCallback = textToSpeechBackend.callback();
        if (jarvisProfile)
        {
            textToSpeechContext = &piper;
            textToSpeechCallback = piper.callback();
        }
        hal::XWalkTextToSpeech textToSpeech(boardControl, textToSpeechContext, textToSpeechCallback);
        hal::XWalkLanguageModelHttp modelBackend(
            modelDialect, modelEndpoint, modelName, modelApiKey, modelTimeoutMs, maximumOutputTokens);
        hal::XWalkLanguageModel languageModel(&modelBackend, modelBackend.callbacks());
        languageModel.setMaximumMessages(maximumMessages);
        hal::XWalkVoiceAssistantConfiguration assistantConfiguration{};
        if (jarvisProfile)
        {
            assistantConfiguration = XWalkVoiceActiveCarGpt::assistantConfiguration();
        }
        else if (mode == XWALK_BOOT_GPT_CAR_REQ)
        {
            assistantConfiguration = XWalkGptCar::assistantConfiguration();
        }
        else
        {
            assistantConfiguration = XWalkVoiceActiveCar::assistantConfiguration();
        }
        XWalkVoiceActiveCarConfiguration voiceConfiguration{};
        if (jarvisProfile)
        {
            voiceConfiguration = XWalkVoiceActiveCarGpt::carConfiguration();
            const agent::boolean configuredWithImage = parseBoolean(
                config.get("voice_active_car_gpt_with_image", XWalkVoiceActiveCarGpt::WITH_IMAGE ? "true" : "false"),
                "voice_active_car_gpt_with_image");
            if (configuredWithImage)
            {
                XWALK_RPIAGENT_ERROR(XWALK_INVAL, "voice_active_car_gpt_with_image must remain false");
            }
            voiceConfiguration.withImage = configuredWithImage;
            voiceConfiguration.continuousConversationEnabled =
                parseBoolean(config.get("voice_active_car_gpt_continuous_conversation",
                                        XWalkVoiceActiveCarGpt::CONTINUOUS_CONVERSATION ? "true" : "false"),
                             "voice_active_car_gpt_continuous_conversation");
            voiceConfiguration.conversationIdleTimeoutMs =
                parseUnsigned(config.get("voice_active_car_gpt_conversation_idle_timeout_ms",
                                         std::to_string(XWalkVoiceActiveCarGpt::CONVERSATION_IDLE_TIMEOUT_MS)),
                              "voice_active_car_gpt_conversation_idle_timeout_ms",
                              300'000U);
            voiceConfiguration.conversationMaximumRounds =
                parseUnsigned(config.get("voice_active_car_gpt_conversation_maximum_rounds",
                                         std::to_string(XWalkVoiceActiveCarGpt::CONVERSATION_MAXIMUM_ROUNDS)),
                              "voice_active_car_gpt_conversation_maximum_rounds",
                              100U);
            voiceConfiguration.conversationMaximumMisses =
                parseUnsigned(config.get("voice_active_car_gpt_conversation_maximum_misses",
                                         std::to_string(XWalkVoiceActiveCarGpt::CONVERSATION_MAXIMUM_MISSES)),
                              "voice_active_car_gpt_conversation_maximum_misses",
                              10U);
            voiceConfiguration.sleepPhrases =
                parsePhraseList(config.get("voice_active_car_gpt_sleep_phrases", XWalkVoiceActiveCarGpt::SLEEP_PHRASES),
                                "voice_active_car_gpt_sleep_phrases");
            voiceConfiguration.sleepAcknowledgement =
                config.get("voice_active_car_gpt_sleep_acknowledgement", XWalkVoiceActiveCarGpt::SLEEP_ACKNOWLEDGEMENT);
            voiceConfiguration.webSearchEnabled =
                parseBoolean(config.get("voice_active_car_gpt_web_search_enabled",
                                        XWalkVoiceActiveCarGpt::WEB_SEARCH_ENABLED ? "true" : "false"),
                             "voice_active_car_gpt_web_search_enabled");
        }
        else if (mode == XWALK_BOOT_GPT_CAR_REQ)
        {
            voiceConfiguration = XWalkGptCar::carConfiguration();
        }
        else
        {
            voiceConfiguration = XWalkVoiceActiveCar::carConfiguration();
        }
        hal::XWalkVoiceAssistant voiceAssistant(speechToText, languageModel, textToSpeech, assistantConfiguration);

        hal::XWalkMusicAlsa musicBackend(audioBackend, nullptr, hal::XWalkMusicSndFileDecoder::operations());
        hal::XWalkMusic music(&musicBackend, musicBackend.callbacks());
        XWalkSelfDrive selfDrive(picarx,
                                 music,
                                 nullptr,
                                 &selfDriveDelayMilliseconds,
                                 nullptr,
                                 config.get("resource_sound_directory", "/usr/share/xwalk/sounds"),
                                 config.get("resource_music_directory", "/usr/share/xwalk/music"));
        const agent::string gpioPath(parameters.gpioDevice);
        const agent::string chipName(parameters.chipName);
        const agent::string chipLabel(parameters.chipLabel);
        hal::XWalkGpioLinux ledBackend(gpioPath.c_str(), chipName, chipLabel, parameters.minLines);
        hal::XWalkGpio ledGpio(&ledBackend, *parameters.gpioOps, config.get("hardware_status_led_pin", "LED"));
        hal::XWalkLed statusLed(ledGpio);
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.voiceAssistant = &voiceAssistant;
        services.selfDrive = &selfDrive;
        services.music = &music;
        services.voiceStatusLed = &statusLed;
        services.voiceActiveCarConfiguration = &voiceConfiguration;
        if (jarvisProfile)
        {
            if (voiceConfiguration.webSearchEnabled)
            {
                hal::XWalkWebSearchConfiguration webSearchConfiguration{};
                webSearchConfiguration.endpoint =
                    config.get("voice_active_car_gpt_web_search_endpoint", XWalkVoiceActiveCarGpt::WEB_SEARCH_ENDPOINT);
                webSearchConfiguration.maximumResults =
                    parseUnsigned(config.get("voice_active_car_gpt_web_search_maximum_results",
                                             std::to_string(XWalkVoiceActiveCarGpt::WEB_SEARCH_MAXIMUM_RESULTS)),
                                  "voice_active_car_gpt_web_search_maximum_results",
                                  10U);
                webSearchConfiguration.timeoutMs =
                    parseUnsigned(config.get("voice_active_car_gpt_web_search_timeout_ms",
                                             std::to_string(XWalkVoiceActiveCarGpt::WEB_SEARCH_TIMEOUT_MS)),
                                  "voice_active_car_gpt_web_search_timeout_ms",
                                  30'000U);
                webSearchConfiguration.maximumResponseBytes =
                    parseUnsigned(config.get("voice_active_car_gpt_web_search_maximum_response_bytes",
                                             std::to_string(XWalkVoiceActiveCarGpt::WEB_SEARCH_MAXIMUM_RESPONSE_BYTES)),
                                  "voice_active_car_gpt_web_search_maximum_response_bytes",
                                  1'048'576U);
                hal::XWalkWebSearch webSearch(webSearchConfiguration);
                services.webSearch = &webSearch;
                return parameters.callback(parameters.appContext, services);
            }
            return parameters.callback(parameters.appContext, services);
        }
        if (voiceConfiguration.withImage == false)
        {
            return parameters.callback(parameters.appContext, services);
        }

        const hal::XWalkCameraConnection cameraConnection =
            hal::XWalkCamera::connectionFromString(config.get("camera_connection", "csi"));
        const agent::boolean csiSelected =
            static_cast<agent::boolean>(cameraConnection == hal::XWalkCameraConnection::Csi);
        const agent::string cameraExecutable = csiSelected ? config.get("camera_csi_executable", "rpicam-still")
                                                           : config.get("camera_usb_executable", "ffmpeg");
        hal::XWalkCameraLinux cameraBackend(
            cameraConnection, cameraExecutable, config.get("camera_usb_device", "/dev/video0"));
        hal::XWalkCameraConfiguration cameraConfiguration;
        cameraConfiguration.widthPixels = parseUnsigned(config.get("camera_width", "640"), "camera_width", 7'680U);
        cameraConfiguration.heightPixels = parseUnsigned(config.get("camera_height", "480"), "camera_height", 4'320U);
        cameraConfiguration.timeoutMs =
            parseUnsigned(config.get("camera_timeout_ms", "5000"), "camera_timeout_ms", 300'000U);
        hal::XWalkCamera camera(&cameraBackend, cameraBackend.callback(), cameraConfiguration);
        XWalkCameraCapture cameraCapture(camera, config.get("camera_output", "/tmp/xwalk-voice-image.jpg"));
        services.cameraCapture = &cameraCapture;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
