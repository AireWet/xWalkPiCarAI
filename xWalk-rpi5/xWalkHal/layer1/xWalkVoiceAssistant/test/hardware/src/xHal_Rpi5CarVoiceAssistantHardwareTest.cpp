/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantHardwareTest.cpp
 * @brief       Provides an explicit full voice-backend integration smoke test.
 *
 * @details
 * Captures bounded ALSA microphone PCM, binds it to an approved prompt, sends
 * one Ollama request, and plays a deployment-owned PCM fixture through ALSA.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant Hardware Test
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

#include "xHal_Rpi5CarAudioAlsa.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTextToSpeechAlsa.h"

#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarVoiceAssistantHardwareTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using SmokeProviders = ::xwalk::source_types::xhal_rpi5carvoiceassistanthardwaretest::SmokeProviders;
/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains explicit provider state and non-physical board-control
 * seams. */
namespace
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /******************************************************************************
     * Private function definitions
     ******************************************************************************/

    /** @brief Retains the non-physical GPIO configuration used by board control. */
    void configureGpio(XWalkHal::contextpointer context,
                       XWalkHal::uint8 pin,
                       XWalkHal::XWalkGpioMode mode,
                       XWalkHal::XWalkGpioPull pull,
                       XWalkHal::boolean initialValue)
    {
        static_cast<SmokeProviders*>(context)->gpioLevel = initialValue;
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
    }

    /** @brief Returns the retained non-physical GPIO level. */
    XWalkHal::boolean readGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<SmokeProviders*>(context)->gpioLevel;
    }

    /** @brief Retains one non-physical GPIO output level. */
    void writeGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value)
    {
        static_cast<SmokeProviders*>(context)->gpioLevel = value;
        static_cast<void>(pin);
    }

    /** @brief Accepts an unused non-physical interrupt registration. */
    void registerInterrupt(XWalkHal::contextpointer context,
                           XWalkHal::uint8 pin,
                           XWalkHal::XWalkGpioEdge edge,
                           XWalkHal::uint32 debounceMs,
                           XWalkHal::contextpointer handlerContext,
                           XWalkHal::gpiointerrupthandler handler)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(edge);
        static_cast<void>(debounceMs);
        static_cast<void>(handlerContext);
        static_cast<void>(handler);
    }

    /** @brief Accepts an unused non-physical interrupt cancellation. */
    void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }

    /** @brief Returns the complete non-physical GPIO operation table. */
    XWalkHal::XWalkGpioCallbacks gpioOperations()
    {
        return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
    }

    /** @brief Reports the non-physical board-control I2C device as present. */
    XWalkHal::boolean probeI2c(XWalkHal::contextpointer context, XWalkHal::uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }

    /** @brief Accepts an unused non-physical board-control I2C write. */
    void writeI2c(XWalkHal::contextpointer context,
                  XWalkHal::uint8 address,
                  XWalkHal::uint8 reg,
                  const XWalkHal::bytevector& data)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
    }

    /** @brief Returns unused non-physical board-control I2C bytes. */
    XWalkHal::bytevector readI2c(XWalkHal::contextpointer context, XWalkHal::uint8 address, XWalkHal::size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(length);
        return {};
    }

    /** @brief Accepts speaker priming because power is deployment-owned in this
     * smoke test. */
    void primeSpeaker(XWalkHal::contextpointer context, XWalkHal::uint32 durationMs)
    {
        static_cast<void>(context);
        static_cast<void>(durationMs);
    }

    /** @brief Reports readiness of the explicit prompt-binding recognizer. */
    XWalkHal::boolean recognizerReady(XWalkHal::contextpointer context)
    {
        return !static_cast<SmokeProviders*>(context)->prompt.empty();
    }

    /** @brief Validates captured PCM and returns the deployment-approved prompt. */
    XWalkHal::string recognizePcm(XWalkHal::contextpointer context,
                                  const XWalkHal::bytevector& pcmData,
                                  XWalkHal::uint32 sampleRateHz,
                                  XWalkHal::uint8 channelCount)
    {
        const hal::boolean pcmDataSampleRateHzChannelCountInvalid =
            static_cast<hal::boolean>(pcmData.empty() || (sampleRateHz != 16'000U) || (channelCount != 1U));
        if (pcmDataSampleRateHzChannelCountInvalid)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Voice-assistant smoke test received invalid microphone PCM");
        }
        return static_cast<SmokeProviders*>(context)->prompt;
    }

    /** @brief Starts one prompt-backed streaming recognition session. */
    XWalkHal::speechrecognitionsession
    startRecognition(XWalkHal::contextpointer context, XWalkHal::uint32 sampleRateHz, XWalkHal::uint8 channelCount)
    {
        const hal::boolean formatInvalid = static_cast<hal::boolean>((sampleRateHz != 16'000U) || (channelCount != 1U));
        if (formatInvalid)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Voice-assistant smoke streaming PCM format is invalid");
        }
        return context;
    }

    /** @brief Treats the first captured period as one complete approved prompt. */
    XWalkHal::XWalkSpeechRecognitionFeedStatus feedRecognition(XWalkHal::contextpointer context,
                                                               XWalkHal::speechrecognitionsession session,
                                                               const XWalkHal::bytevector& pcmData)
    {
        const hal::boolean periodInvalid = static_cast<hal::boolean>((session != context) || pcmData.empty());
        if (periodInvalid)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Voice-assistant smoke streaming PCM period is invalid");
        }
        return XWalkHal::XWalkSpeechRecognitionFeedStatus::Endpoint;
    }

    /** @brief Returns the deployment-approved prompt after endpoint finalization. */
    XWalkHal::string finishRecognition(XWalkHal::contextpointer context,
                                       XWalkHal::speechrecognitionsession session,
                                       XWalkHal::boolean endpointDetected)
    {
        const hal::boolean finalizationInvalid = static_cast<hal::boolean>((session != context) || !endpointDetected);
        if (finalizationInvalid)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Voice-assistant smoke streaming finalization is invalid");
        }
        return static_cast<SmokeProviders*>(context)->prompt;
    }

    /** @brief Releases the context-backed prompt session. */
    void releaseRecognition(XWalkHal::contextpointer context, XWalkHal::speechrecognitionsession session)
    {
        static_cast<void>(context);
        static_cast<void>(session);
    }

    /** @brief Returns silence for the unused file-recognition path. */
    XWalkHal::string recognizeFile(XWalkHal::contextpointer context, XWalkHal::stringview filePath)
    {
        static_cast<void>(context);
        static_cast<void>(filePath);
        return {};
    }

    /** @brief Accepts repeated recognition cancellation. */
    void cancelRecognition(XWalkHal::contextpointer context)
    {
        static_cast<void>(context);
    }

    /** @brief Loads deployment-owned signed-sixteen mono PCM for model responses.
     */
    XWalkHal::XWalkTextToSpeechPcmData synthesizeFixture(XWalkHal::contextpointer context, XWalkHal::stringview text)
    {
        const hal::boolean textEmpty = static_cast<hal::boolean>(text.empty());
        if (textEmpty)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Voice-assistant smoke response must not be empty");
        }
        const SmokeProviders& providers = *static_cast<SmokeProviders*>(context);
        const XWalkHal::string contents = XWalkHal::readFileContents(providers.synthesisFixture);
        const hal::boolean contentsInvalid =
            static_cast<hal::boolean>(contents.empty() || ((contents.size() % 2U) != 0U));
        if (contentsInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Voice-assistant smoke fixture must contain complete mono PCM frames");
        }
        XWalkHal::bytevector pcmData{};
        pcmData.reserve(contents.size());
        for (const char character : contents)
        {
            pcmData.push_back(static_cast<XWalkHal::uint8>(static_cast<unsigned char>(character)));
        }
        return {pcmData, 16'000U, 1U};
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Composes completed speech and model backends for one bounded round.
 *
 * @param[in] argumentCount Exactly nine arguments are required.
 * @param[in] argumentValues Program, capture, PCM, mixer, element, endpoint,
 * model, approved prompt, and raw PCM response fixture.
 * @return Zero after one successful capture, model request, and playback.
 * @warning Run only after microphone privacy, network, model, prompt, speaker
 * power, playback device, volume, and fixture content are explicitly approved.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer argumentValues[])
{
    if (argumentCount != 9)
    {
        XWALK_HAL_ERROR(XWALK_INVAL, "Voice-assistant hardware test requires eight explicit settings");
    }

    SmokeProviders providers{argumentValues[7], argumentValues[8], false};
    const hal::boolean promptEmpty = static_cast<hal::boolean>(providers.prompt.empty());
    if (promptEmpty)
    {
        XWALK_HAL_ERROR(XWALK_INVAL, "Voice-assistant hardware prompt must not be empty");
    }
    const XWalkHal::XWalkGpioCallbacks gpioCallbacks = gpioOperations();
    XWalkHal::XWalkGpio resetGpio(&providers, gpioCallbacks, "MCURST");
    XWalkHal::XWalkGpio speakerGpio(&providers, gpioCallbacks, XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
    XWalkHal::XWalkI2c i2c(&providers, &probeI2c, &writeI2c, &readI2c);
    XWalkHal::XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc, nullptr, &primeSpeaker);
    XWalkHal::XWalkAudioAlsa audio(argumentValues[2], argumentValues[3], argumentValues[4]);
    XWalkHal::XWalkSpeechToTextAlsaOperations recognizerOperations{};
    recognizerOperations.recognizerReady = &recognizerReady;
    recognizerOperations.recognizePcm = &recognizePcm;
    recognizerOperations.startRecognition = &startRecognition;
    recognizerOperations.feedRecognition = &feedRecognition;
    recognizerOperations.finishRecognition = &finishRecognition;
    recognizerOperations.releaseRecognition = &releaseRecognition;
    recognizerOperations.recognizeFile = &recognizeFile;
    recognizerOperations.cancelRecognition = &cancelRecognition;
    XWalkHal::XWalkSpeechToTextAlsa speechBackend(argumentValues[1], &providers, recognizerOperations);
    XWalkHal::XWalkSpeechToText speechToText(&speechBackend, speechBackend.callbacks());
    XWalkHal::XWalkLanguageModelOllama modelBackend(argumentValues[5], argumentValues[6]);
    XWalkHal::XWalkLanguageModel languageModel(&modelBackend, modelBackend.callbacks());
    const XWalkHal::XWalkTextToSpeechAlsaOperations synthesisOperations{&synthesizeFixture};
    XWalkHal::XWalkTextToSpeechAlsa speechOutputBackend(audio, &providers, synthesisOperations, 15U);
    XWalkHal::XWalkTextToSpeech textToSpeech(boardControl, &speechOutputBackend, speechOutputBackend.callback());
    XWalkHal::XWalkVoiceAssistant assistant(speechToText, languageModel, textToSpeech, {"Answer briefly.", {}});

    assistant.start();
    const XWalkHal::string response = assistant.runRound(100U);
    assistant.stop();
    const hal::boolean responseEmpty = static_cast<hal::boolean>(response.empty());
    if (responseEmpty)
    {
        XWALK_HAL_ERROR(XWALK_RUNTIME, "Voice-assistant hardware response is empty");
    }
    return 0;
}
