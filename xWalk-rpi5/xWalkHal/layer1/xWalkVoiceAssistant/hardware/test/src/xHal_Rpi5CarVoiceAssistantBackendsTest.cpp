/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantBackendsTest.cpp
 * @brief       Verifies complete voice-backend composition without devices.
 *
 * @details
 * Composes injected ALSA capture and playback, Ollama transport, Robot HAT
 * speaker control, and the synchronous voice-assistant coordinator.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant Backend Host Test
 *
 * @author      Joxy John
 * @date        2026-08-01
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
#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTextToSpeechAlsa.h"
#include "xHal_Rpi5CarVoiceAssistantBackendsTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TestBackends = ::xwalk::source_types::xhal_rpi5carvoiceassistantbackendstest::TestBackends;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains deterministic full-stack backend operations. */
namespace
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /******************************************************************************
     * Private function definitions
     ******************************************************************************/

    /** @brief Retains one simulated GPIO configuration. */
    void configureGpio(XWalkHal::contextpointer context,
                       XWalkHal::uint8 pin,
                       XWalkHal::XWalkGpioMode mode,
                       XWalkHal::XWalkGpioPull pull,
                       XWalkHal::boolean initialValue)
    {
        static_cast<TestBackends*>(context)->gpioLevel = initialValue;
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
    }

    /** @brief Returns the simulated GPIO level. */
    XWalkHal::boolean readGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<TestBackends*>(context)->gpioLevel;
    }

    /** @brief Retains one simulated GPIO output level. */
    void writeGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value)
    {
        static_cast<TestBackends*>(context)->gpioLevel = value;
        static_cast<void>(pin);
    }

    /** @brief Accepts an unused simulated interrupt registration. */
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

    /** @brief Accepts an unused simulated interrupt cancellation. */
    void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }

    /** @brief Returns the complete simulated GPIO table. */
    XWalkHal::XWalkGpioCallbacks gpioOperations()
    {
        return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
    }

    /** @brief Reports the simulated I2C device as present. */
    XWalkHal::boolean probeI2c(XWalkHal::contextpointer context, XWalkHal::uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }

    /** @brief Accepts an unused simulated I2C write. */
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

    /** @brief Returns unused simulated I2C bytes. */
    XWalkHal::bytevector readI2c(XWalkHal::contextpointer context, XWalkHal::uint8 address, XWalkHal::size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(length);
        return {};
    }

    /** @brief Records one speaker-prime request. */
    void primeSpeaker(XWalkHal::contextpointer context, XWalkHal::uint32 durationMs)
    {
        ++static_cast<TestBackends*>(context)->primeCount;
        assert(durationMs == XHAL_RPI5CAR_BOARD_CONTROL_SPEAKER_PRIME_DURATION_MS);
    }

    /** @brief Opens one simulated microphone capture handle. */
    XWalkHal::speechcapturehandle openCapture(XWalkHal::contextpointer context,
                                              XWalkHal::stringview deviceName,
                                              XWalkHal::uint32 sampleRateHz,
                                              XWalkHal::uint8 channelCount,
                                              XWalkHal::uint32 periodFrames)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(deviceName == "test-microphone");
        assert(sampleRateHz == 16'000U && channelCount == 1U && periodFrames == 1'024U);
        return &state.captureToken;
    }

    /** @brief Supplies one complete bounded microphone capture period. */
    XWalkHal::int32 readCapture(XWalkHal::contextpointer context,
                                XWalkHal::speechcapturehandle captureHandle,
                                XWalkHal::bytevector& pcmData,
                                XWalkHal::size frameCount)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(captureHandle == &state.captureToken);
        pcmData.assign(frameCount * 2U, 0x11U);
        state.capturedBytes += pcmData.size();
        return static_cast<XWalkHal::int32>(frameCount);
    }

    /** @brief Rejects capture recovery because the simulated read never fails. */
    XWalkHal::boolean recoverCapture(XWalkHal::contextpointer context,
                                     XWalkHal::speechcapturehandle captureHandle,
                                     XWalkHal::int32 errorValue)
    {
        static_cast<void>(context);
        static_cast<void>(captureHandle);
        static_cast<void>(errorValue);
        return false;
    }

    /** @brief Accepts closure of the simulated capture handle. */
    void closeCapture(XWalkHal::contextpointer context, XWalkHal::speechcapturehandle captureHandle)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(captureHandle == &state.captureToken);
    }

    /** @brief Reports that the simulated recognizer is ready. */
    XWalkHal::boolean recognizerReady(XWalkHal::contextpointer context)
    {
        static_cast<void>(context);
        return true;
    }

    /** @brief Converts captured PCM into one deterministic transcript. */
    XWalkHal::string recognizePcm(XWalkHal::contextpointer context,
                                  const XWalkHal::bytevector& pcmData,
                                  XWalkHal::uint32 sampleRateHz,
                                  XWalkHal::uint8 channelCount)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(!pcmData.empty() && sampleRateHz == 16'000U && channelCount == 1U);
        ++state.recognitionCount;
        return "where am I";
    }

    /** @brief Returns silence for the unused file-recognition path. */
    XWalkHal::string recognizeFile(XWalkHal::contextpointer context, XWalkHal::stringview filePath)
    {
        static_cast<void>(context);
        static_cast<void>(filePath);
        return {};
    }

    /** @brief Accepts repeated simulated recognition cancellation. */
    void cancelRecognition(XWalkHal::contextpointer context)
    {
        static_cast<void>(context);
    }

    /** @brief Returns complete injected capture and recognition operations. */
    XWalkHal::XWalkSpeechToTextAlsaOperations speechOperations()
    {
        return {&openCapture,
                &readCapture,
                &recoverCapture,
                &closeCapture,
                &recognizerReady,
                &recognizePcm,
                &recognizeFile,
                &cancelRecognition};
    }

    /** @brief Records one Ollama request and returns a final assistant response. */
    XWalkHal::string postModel(XWalkHal::contextpointer context,
                               XWalkHal::stringview endpoint,
                               XWalkHal::stringview requestJson,
                               XWalkHal::stringview authorizationHeader,
                               XWalkHal::uint32 timeoutMs,
                               XWalkHal::size maximumResponseBytes)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(endpoint == "http://localhost:11434/api/chat");
        assert(authorizationHeader.empty());
        assert(timeoutMs > 0U && maximumResponseBytes > 0U);
        state.modelRequest = requestJson;
        ++state.modelCount;
        return "{\"message\":{\"role\":\"assistant\",\"content\":\"You are here.\"}}";
    }

    /** @brief Synthesizes deterministic mono signed-sixteen PCM. */
    XWalkHal::XWalkTextToSpeechPcmData synthesize(XWalkHal::contextpointer context, XWalkHal::stringview text)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        state.synthesizedText = text;
        ++state.synthesisCount;
        return {XWalkHal::bytevector(320U, 0x22U), 16'000U, 1U};
    }

    /** @brief Opens one simulated PCM playback handle. */
    XWalkHal::audiopcmhandle openPcm(XWalkHal::contextpointer context, XWalkHal::stringview deviceName)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(deviceName == "test-pcm");
        return &state.pcmToken;
    }

    /** @brief Accepts one valid simulated PCM stream configuration. */
    XWalkHal::boolean configurePcm(XWalkHal::contextpointer context,
                                   XWalkHal::audiopcmhandle pcmHandle,
                                   const XWalkHal::XWalkAudioStreamConfiguration& configuration)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(pcmHandle == &state.pcmToken);
        assert(configuration.sampleRateHz == 16'000U && configuration.channelCount == 1U);
        return true;
    }

    /** @brief Records one complete simulated PCM write. */
    XWalkHal::int32 writePcm(XWalkHal::contextpointer context,
                             XWalkHal::audiopcmhandle pcmHandle,
                             const XWalkHal::bytevector& pcmData,
                             XWalkHal::size byteOffset,
                             XWalkHal::size frameCount)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(pcmHandle == &state.pcmToken && byteOffset == 0U);
        state.playedBytes += pcmData.size();
        return static_cast<XWalkHal::int32>(frameCount);
    }

    /** @brief Rejects playback recovery because simulated writes never fail. */
    XWalkHal::boolean
    recoverPcm(XWalkHal::contextpointer context, XWalkHal::audiopcmhandle pcmHandle, XWalkHal::int32 errorValue)
    {
        static_cast<void>(context);
        static_cast<void>(pcmHandle);
        static_cast<void>(errorValue);
        return false;
    }

    /** @brief Records closure of one simulated PCM stream. */
    void closePcm(XWalkHal::contextpointer context, XWalkHal::audiopcmhandle pcmHandle)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(pcmHandle == &state.pcmToken);
        ++state.streamCloseCount;
    }

    /** @brief Opens one simulated persistent mixer. */
    XWalkHal::audiomixerhandle openMixer(XWalkHal::contextpointer context, XWalkHal::stringview deviceName)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(deviceName == "test-mixer");
        return &state.mixerToken;
    }

    /** @brief Accepts one simulated mixer-volume update. */
    XWalkHal::boolean setMixerVolume(XWalkHal::contextpointer context,
                                     XWalkHal::audiomixerhandle mixerHandle,
                                     XWalkHal::stringview elementName,
                                     XWalkHal::uint8 volumePercent)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(mixerHandle == &state.mixerToken && elementName == "PCM");
        assert(volumePercent == 15U);
        return true;
    }

    /** @brief Accepts closure of the simulated persistent mixer. */
    void closeMixer(XWalkHal::contextpointer context, XWalkHal::audiomixerhandle mixerHandle)
    {
        TestBackends& state = *static_cast<TestBackends*>(context);
        assert(mixerHandle == &state.mixerToken);
    }

    /** @brief Returns complete injected shared-audio operations. */
    XWalkHal::XWalkAudioAlsaOperations audioOperations()
    {
        return {&openPcm, &configurePcm, &writePcm, &recoverPcm, &closePcm, &openMixer, &setMixerVolume, &closeMixer};
    }

    /** @brief Composes every concrete adapter and runs one complete round. */
    void testCompletedBackendComposition()
    {
        TestBackends state;
        const XWalkHal::XWalkGpioCallbacks gpioCallbacks = gpioOperations();
        XWalkHal::XWalkGpio resetGpio(&state, gpioCallbacks, "MCURST");
        XWalkHal::XWalkGpio speakerGpio(&state, gpioCallbacks, XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
        XWalkHal::XWalkI2c i2c(&state, &probeI2c, &writeI2c, &readI2c);
        XWalkHal::XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkHal::XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc, &state, &primeSpeaker);
        XWalkHal::XWalkAudioAlsa audio(&state, audioOperations(), "test-pcm", "test-mixer", "PCM");
        XWalkHal::XWalkSpeechToTextAlsa speechBackend(&state, speechOperations(), "test-microphone");
        XWalkHal::XWalkSpeechToText speechToText(&speechBackend, speechBackend.callbacks());
        const XWalkHal::XWalkLanguageModelOllamaOperations modelOperations{&postModel};
        XWalkHal::XWalkLanguageModelOllama modelBackend(
            &state, modelOperations, "http://localhost:11434/api/chat", "test-model");
        XWalkHal::XWalkLanguageModel languageModel(&modelBackend, modelBackend.callbacks());
        const XWalkHal::XWalkTextToSpeechAlsaOperations synthesisOperations{&synthesize};
        XWalkHal::XWalkTextToSpeechAlsa speechOutputBackend(audio, &state, synthesisOperations, 15U);
        XWalkHal::XWalkTextToSpeech textToSpeech(boardControl, &speechOutputBackend, speechOutputBackend.callback());
        const XWalkHal::XWalkVoiceAssistantConfiguration configuration{"Answer briefly.", {}};
        XWalkHal::XWalkVoiceAssistant assistant(speechToText, languageModel, textToSpeech, configuration);

        assistant.start();
        assert(assistant.runRound(100U) == "You are here.");
        assistant.stop();

        assert(state.primeCount == 1U);
        assert(state.recognitionCount == 1U && state.capturedBytes == 3'200U);
        assert(state.modelCount == 1U);
        assert(state.modelRequest.find("where am I") != XWalkHal::string::npos);
        assert(state.modelRequest.find("Answer briefly.") != XWalkHal::string::npos);
        assert(state.synthesisCount == 1U && state.synthesizedText == "You are here.");
        assert(state.playedBytes == 320U && state.streamCloseCount == 1U);
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the device-free completed-backend composition test.
 * @return Zero after every full-stack assertion passes.
 */
XWalkHal::int32 main()
{
    testCompletedBackendComposition();
    return 0;
}
