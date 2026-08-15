/******************************************************************************
 * @file        xHal_Rpi5CarAudioAlsaTestSupport.h
 * @brief       Declares reusable ALSA software-test support.
 *
 * @details
 * Defines injected Audio state and operation factories shared by the ALSA
 * software-test scenarios.
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

#ifndef XHAL_RPI5CAR_AUDIO_ALSA_TEST_SUPPORT_H
#define XHAL_RPI5CAR_AUDIO_ALSA_TEST_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAudioAlsa.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test::audio
 * @brief Contains reusable injected support for Audio host tests.
 */
namespace xwalk::hal::test::audio
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Records every injected ALSA operation and configurable result. */
    struct TestAudioOperations
    {
            /** @brief Stable storage returned as simulated PCM handles. */
            fixedarray<uint32, XHAL_RPI5CAR_AUDIO_MAXIMUM_STREAM_COUNT> pcmTokens{};

            /** @brief Stable storage returned as the simulated mixer handle. */
            uint32 mixerToken{};

            /** @brief Most recently requested PCM device name. */
            string pcmDevice{};

            /** @brief Most recently requested mixer device name. */
            string mixerDevice{};

            /** @brief Most recently requested mixer element name. */
            string mixerElement{};

            /** @brief Most recently negotiated PCM stream configuration. */
            XWalkAudioStreamConfiguration configuration{};

            /** @brief Byte offset observed by the most recent PCM write. */
            size byteOffset{};

            /** @brief Frame count observed by the most recent PCM write. */
            size requestedFrames{};

            /** @brief Number of simulated PCM open operations. */
            uint32 openPcmCount{};

            /** @brief Number of simulated PCM configuration operations. */
            uint32 configureCount{};

            /** @brief Number of simulated PCM write operations. */
            uint32 writeCount{};

            /** @brief Number of simulated PCM recovery operations. */
            uint32 recoverCount{};

            /** @brief Number of simulated PCM close operations. */
            uint32 closePcmCount{};

            /** @brief Number of simulated mixer open operations. */
            uint32 openMixerCount{};

            /** @brief Number of simulated mixer volume operations. */
            uint32 setVolumeCount{};

            /** @brief Number of simulated mixer close operations. */
            uint32 closeMixerCount{};

            /** @brief Most recently requested mixer volume percentage. */
            uint8 volumePercent{};

            /** @brief Configurable result returned by PCM negotiation. */
            boolean configureSucceeds{true};

            /** @brief Configurable result returned by PCM recovery. */
            boolean recoverSucceeds{true};

            /** @brief Configurable result returned by the first PCM write. */
            int32 firstWriteResult{-32};
    };

    /******************************************************************************
     * Function declarations
     ******************************************************************************/

    /**
     * @brief Opens one simulated PCM handle and records the requested device.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] deviceName Non-owning PCM device name retained as an owned copy.
     * @return Non-null simulated PCM handle backed by the test state.
     */
    audiopcmhandle openPcm(contextpointer context, stringview deviceName);

    /**
     * @brief Records one simulated PCM negotiation.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] pcmHandle Simulated PCM handle; not dereferenced.
     * @param[in] configuration Requested stream configuration retained by value.
     * @return Configurable negotiation result from the test state.
     */
    boolean
    configurePcm(contextpointer context, audiopcmhandle pcmHandle, const XWalkAudioStreamConfiguration& configuration);

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
        contextpointer context, audiopcmhandle pcmHandle, const bytevector& pcmData, size byteOffset, size frameCount);

    /**
     * @brief Records one simulated recovery attempt.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] pcmHandle Simulated PCM handle; not dereferenced.
     * @param[in] errorValue Negative PCM error value that requested recovery.
     * @return Configurable recovery result from the test state.
     */
    boolean recoverPcm(contextpointer context, audiopcmhandle pcmHandle, int32 errorValue);

    /**
     * @brief Records one simulated PCM drain-and-close operation.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] pcmHandle Non-null simulated PCM handle.
     */
    void closePcm(contextpointer context, audiopcmhandle pcmHandle);

    /**
     * @brief Opens the simulated persistent mixer handle.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] deviceName Non-owning mixer device name retained as an owned copy.
     * @return Non-null simulated mixer handle backed by the test state.
     */
    audiomixerhandle openMixer(contextpointer context, stringview deviceName);

    /**
     * @brief Records one simulated mixer-element volume update.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] mixerHandle Non-null simulated mixer handle.
     * @param[in] elementName Non-owning mixer element name retained as an owned copy.
     * @param[in] volumePercent Requested volume from zero through one hundred percent.
     * @return Always `true`.
     */
    boolean
    setMixerVolume(contextpointer context, audiomixerhandle mixerHandle, stringview elementName, uint8 volumePercent);

    /**
     * @brief Records closure of the simulated persistent mixer handle.
     * @param[in,out] context Non-null `TestAudioOperations` state.
     * @param[in] mixerHandle Non-null simulated mixer handle.
     */
    void closeMixer(contextpointer context, audiomixerhandle mixerHandle);

    /**
     * @brief Creates the complete injected ALSA operation table.
     * @return Operation table bound to this test-support implementation.
     */
    XWalkAudioAlsaOperations testOperations();

    /**
     * @brief Creates a valid signed sixteen-bit stereo stream configuration.
     * @return Configuration for 44,100-Hertz stereo output with 256-frame periods.
     */
    XWalkAudioStreamConfiguration testConfiguration();

} /* namespace xwalk::hal::test::audio */

#endif /* XHAL_RPI5CAR_AUDIO_ALSA_TEST_SUPPORT_H */
