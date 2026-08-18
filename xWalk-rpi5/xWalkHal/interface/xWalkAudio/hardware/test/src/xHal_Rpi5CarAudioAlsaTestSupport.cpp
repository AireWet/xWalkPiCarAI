/******************************************************************************
 * @file        xHal_Rpi5CarAudioAlsaTestSupport.cpp
 * @brief       Implements reusable ALSA software-test support.
 *
 * @details
 * Supplies deterministic injected operations that exercise Audio ownership,
 * short-write, recovery, and mixer behavior without opening an ALSA device.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio ALSA Software Test
 *
 * @author      Joxy John
 * @date        2026-08-10
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

#include "xHal_Rpi5CarAudioAlsaTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test::audio
 * @brief Contains reusable injected support for Audio host tests.
 */
namespace xwalk::hal::test::audio
{

    /**
     * @brief Opens one simulated PCM handle and records the requested device.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] deviceName Non-owning PCM device name retained as an owned copy.
     * @return Non-null simulated PCM handle backed by the test state.
     */
    audiopcmhandle openPcm(contextpointer context, stringview deviceName)
    {
        TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
        state.pcmDevice = string(deviceName);
        const size tokenIndex = static_cast<size>(state.openPcmCount) % state.pcmTokens.size();
        ++state.openPcmCount;
        return &state.pcmTokens[tokenIndex];
    }

    /**
     * @brief Records one simulated PCM negotiation.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] pcmHandle Simulated PCM handle; not dereferenced.
     * @param[in] configuration Requested stream configuration retained by value.
     * @return Configurable negotiation result from the test state.
     */
    boolean
    configurePcm(contextpointer context, audiopcmhandle pcmHandle, const XWalkAudioStreamConfiguration& configuration)
    {
        static_cast<void>(pcmHandle);
        TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
        ++state.configureCount;
        state.configuration = configuration;
        return state.configureSucceeds;
    }

    /**
     * @brief Simulates one underrun followed by bounded short writes.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] pcmHandle Simulated PCM handle; not dereferenced.
     * @param[in] pcmData PCM payload; not retained or inspected.
     * @param[in] byteOffset Current byte offset recorded for assertions.
     * @param[in] frameCount Requested frame count recorded for assertions.
     * @return Configured first result, then at most two written frames.
     */
    int32 writePcm(
        contextpointer context, audiopcmhandle pcmHandle, const bytevector& pcmData, size byteOffset, size frameCount)
    {
        static_cast<void>(pcmHandle);
        static_cast<void>(pcmData);
        TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
        ++state.writeCount;
        state.byteOffset = byteOffset;
        state.requestedFrames = frameCount;
        if (state.writeCount == 1U)
        {
            return state.firstWriteResult;
        }
        return static_cast<int32>(std::min(frameCount, static_cast<size>(2U)));
    }

    /**
     * @brief Records one simulated recovery attempt.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] pcmHandle Simulated PCM handle; not dereferenced.
     * @param[in] errorValue Negative PCM error value that requested recovery.
     * @return Configurable recovery result from the test state.
     */
    boolean recoverPcm(contextpointer context, audiopcmhandle pcmHandle, int32 errorValue)
    {
        static_cast<void>(pcmHandle);
        xwalk::hal::test::requireTestCondition(errorValue < 0);
        TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
        ++state.recoverCount;
        return state.recoverSucceeds;
    }

    /**
     * @brief Records one simulated PCM drain-and-close operation.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] pcmHandle Non-null simulated PCM handle.
     */
    void closePcm(contextpointer context, audiopcmhandle pcmHandle)
    {
        xwalk::hal::test::requireTestCondition(pcmHandle != nullptr);
        ++static_cast<TestAudioOperations*>(context)->closePcmCount;
    }

    /**
     * @brief Opens the simulated persistent mixer handle.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] deviceName Non-owning mixer device name retained as an owned copy.
     * @return Non-null simulated mixer handle backed by the test state.
     */
    audiomixerhandle openMixer(contextpointer context, stringview deviceName)
    {
        TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
        ++state.openMixerCount;
        state.mixerDevice = string(deviceName);
        return &state.mixerToken;
    }

    /**
     * @brief Records one simulated mixer-element volume update.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] mixerHandle Non-null simulated mixer handle.
     * @param[in] elementName Non-owning mixer element name retained as an owned copy.
     * @param[in] volumePercent Requested volume from zero through one hundred percent.
     * @return Always `true`.
     */
    boolean
    setMixerVolume(contextpointer context, audiomixerhandle mixerHandle, stringview elementName, uint8 volumePercent)
    {
        xwalk::hal::test::requireTestCondition(mixerHandle != nullptr);
        TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
        ++state.setVolumeCount;
        state.mixerElement = string(elementName);
        state.volumePercent = volumePercent;
        return true;
    }

    /**
     * @brief Records closure of the simulated persistent mixer handle.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] mixerHandle Non-null simulated mixer handle.
     */
    void closeMixer(contextpointer context, audiomixerhandle mixerHandle)
    {
        xwalk::hal::test::requireTestCondition(mixerHandle != nullptr);
        ++static_cast<TestAudioOperations*>(context)->closeMixerCount;
    }

    /**
     * @brief Creates the complete injected ALSA operation table.
     * @return Operation table bound to this test-support implementation.
     */
    XWalkAudioAlsaOperations testOperations()
    {
        return {&openPcm, &configurePcm, &writePcm, &recoverPcm, &closePcm, &openMixer, &setMixerVolume, &closeMixer};
    }

    /**
     * @brief Creates a valid signed sixteen-bit stereo stream configuration.
     * @return Configuration for 44,100-Hertz stereo output with 256-frame periods.
     */
    XWalkAudioStreamConfiguration testConfiguration()
    {
        return {44'100U, 2U, XWalkAudioSampleFormat::Signed16LittleEndian, 256U, XHAL_RPI5CAR_AUDIO_DEFAULT_LATENCY_US};
    }

} /* namespace xwalk::hal::test::audio */
