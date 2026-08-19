/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVoiceActiveMode.cpp
 * @brief       Composes shared Raspberry Pi voice-active vehicle services.
 *
 * @details
 * Loads speech, model, audio, status-LED, camera, and credential-environment
 * selections before publishing one profile-specific service graph.
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
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] boardControl Caller-owned board controller.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @param[in] gpioDevice Configured GPIO device path.
     * @param[in] gpioChipName Optional exact GPIO chip name.
     * @param[in] gpioChipLabel Optional exact GPIO chip label.
     * @param[in] minimumGpioLineCount Required minimum GPIO line count.
     * @param[in] gpioCallbacks Linux GPIO callback table.
     * @return Status returned by `callback`.
     * @throws std::runtime_error If the selected credential environment is empty.
     */
    agent::int32 XWalkBootRpi::runVoiceActiveMode(agent::uint8 mode,
                                                  agent::contextpointer context,
                                                  bootapplicationcallback callback,
                                                  hal::XWalkConfigStore& config,
                                                  hal::XWalkBoardControl& boardControl,
                                                  XWalkPicarx& picarx,
                                                  agent::stringview gpioDevice,
                                                  agent::stringview gpioChipName,
                                                  agent::stringview gpioChipLabel,
                                                  agent::uint32 minimumGpioLineCount,
                                                  const hal::XWalkGpioCallbacks& gpioCallbacks)
    {
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

        agent::string profileApiKeyEnvironment;
        const agent::boolean rollyProfile = static_cast<agent::boolean>(mode == XWALK_BOOT_VOICE_ACTIVE_CAR_REQ);
        const agent::boolean jarvisProfile = static_cast<agent::boolean>(mode == XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ);
        if (rollyProfile)
        {
            profileApiKeyEnvironment = config.get("voice_active_car_api_key_environment", "OPENAI_API_KEY");
            modelEndpoint = config.get("voice_active_car_endpoint", XWalkVoiceActiveCar::MODEL_ENDPOINT);
            modelName = config.get("voice_active_car_model", XWalkVoiceActiveCar::MODEL_NAME);
            maximumOutputTokensText = config.get("voice_active_car_maximum_output_tokens", "1024");
        }
        else if (jarvisProfile)
        {
            profileApiKeyEnvironment =
                config.get("voice_active_car_gpt_api_key_environment", XWalkVoiceActiveCarGpt::API_KEY_ENVIRONMENT);
            modelEndpoint = config.get("voice_active_car_gpt_endpoint", XWalkVoiceActiveCarGpt::MODEL_ENDPOINT);
            modelName = config.get("voice_active_car_gpt_model", XWalkVoiceActiveCarGpt::MODEL_NAME);
            maximumOutputTokensText = config.get("voice_active_car_gpt_maximum_output_tokens", "1024");
        }
        else
        {
            profileApiKeyEnvironment = config.get("gpt_car_api_key_environment", "OPENAI_API_KEY");
            modelEndpoint = config.get("gpt_car_endpoint", XWalkGptCar::MODEL_ENDPOINT);
            modelName = config.get("gpt_car_model", XWalkGptCar::MODEL_NAME);
            maximumOutputTokensText = config.get("gpt_car_maximum_output_tokens", "1024");
        }
        const agent::cstring profileApiKey = std::getenv(profileApiKeyEnvironment.c_str());
        const agent::boolean profileApiKeyMissing =
            static_cast<agent::boolean>((profileApiKey == nullptr) || (profileApiKey[0U] == '\0'));
        if (profileApiKeyMissing)
        {
            const std::string exceptionMessage =
                std::string(profileApiKeyEnvironment).append(" must be set for the selected voice-active mode");
            XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, exceptionMessage);
        }
        modelProvider = "openai";
        modelApiKey = profileApiKey;

        const agent::uint32 maximumOutputTokens = parseUnsigned(maximumOutputTokensText,
                                                                "voice_language_model_maximum_output_tokens",
                                                                XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_MAXIMUM_OUTPUT_TOKENS);
        const agent::uint32 modelTimeoutMs = parseUnsigned(config.get("voice_language_model_timeout_ms", "120000"),
                                                           "voice_language_model_timeout_ms",
                                                           XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_TIMEOUT_MS);
        const hal::XWalkLanguageModelHttpDialect modelDialect =
            hal::XWalkLanguageModelHttp::dialectFromString(modelProvider);

        hal::XWalkSpeechRecognizerVosk recognizer(voskLibrary, voskModel);
        hal::XWalkSpeechToTextAlsa speechToTextBackend(captureDevice, &recognizer, recognizer.operations());
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
        const agent::string gpioDeviceValue(gpioDevice);
        const agent::string gpioChipNameValue(gpioChipName);
        const agent::string gpioChipLabelValue(gpioChipLabel);
        hal::XWalkGpioLinux ledBackend(
            gpioDeviceValue.c_str(), gpioChipNameValue, gpioChipLabelValue, minimumGpioLineCount);
        hal::XWalkGpio ledGpio(&ledBackend, gpioCallbacks, config.get("hardware_status_led_pin", "LED"));
        hal::XWalkLed statusLed(ledGpio);
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
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.voiceAssistant = &voiceAssistant;
        services.selfDrive = &selfDrive;
        services.music = &music;
        services.voiceStatusLed = &statusLed;
        services.cameraCapture = &cameraCapture;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
