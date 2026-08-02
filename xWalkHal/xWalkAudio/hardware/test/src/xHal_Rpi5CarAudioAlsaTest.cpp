/******************************************************************************
 * @file        xHal_Rpi5CarAudioAlsaTest.cpp
 * @brief       Verifies shared ALSA ownership with injected software operations.
 *
 * @details
 * Exercises negotiation, mixer ownership, short writes, bounded recovery,
 * stream limits, validation, explicit close, and deterministic destruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio ALSA Software Test
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

#include "xHal_Rpi5CarAudioAlsa.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains injected ALSA state and software tests private to this translation unit. */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records every injected ALSA operation and configurable result. */
struct TestAudioOperations
{
    fixedarray<uint32, XHAL_RPI5CAR_AUDIO_MAXIMUM_STREAM_COUNT> pcmTokens{};
    uint32 mixerToken{};
    string pcmDevice{};
    string mixerDevice{};
    string mixerElement{};
    XWalkAudioStreamConfiguration configuration{};
    size byteOffset{};
    size requestedFrames{};
    uint32 openPcmCount{};
    uint32 configureCount{};
    uint32 writeCount{};
    uint32 recoverCount{};
    uint32 closePcmCount{};
    uint32 openMixerCount{};
    uint32 setVolumeCount{};
    uint32 closeMixerCount{};
    uint8 volumePercent{};
    boolean configureSucceeds{true};
    boolean recoverSucceeds{true};
    int32 firstWriteResult{-32};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Opens one simulated PCM handle and records the requested device. */
audiopcmhandle openPcm(contextpointer context, stringview deviceName)
{
    TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
    state.pcmDevice = string(deviceName);
    const size tokenIndex = static_cast<size>(state.openPcmCount) % state.pcmTokens.size();
    ++state.openPcmCount;
    return &state.pcmTokens[tokenIndex];
}

/** @brief Records one simulated PCM negotiation and returns its configured result. */
boolean configurePcm(contextpointer context, audiopcmhandle pcmHandle,
    const XWalkAudioStreamConfiguration& configuration)
{
    static_cast<void>(pcmHandle);
    TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
    ++state.configureCount;
    state.configuration = configuration;
    return state.configureSucceeds;
}

/** @brief Simulates one underrun followed by a short and final PCM write. */
int32 writePcm(contextpointer context, audiopcmhandle pcmHandle,
    const bytevector& pcmData, size byteOffset, size frameCount)
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

/** @brief Records one simulated recovery attempt and returns its configured result. */
boolean recoverPcm(contextpointer context, audiopcmhandle pcmHandle, int32 errorValue)
{
    static_cast<void>(pcmHandle);
    assert(errorValue < 0);
    TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
    ++state.recoverCount;
    return state.recoverSucceeds;
}

/** @brief Records one simulated PCM drain-and-close operation. */
void closePcm(contextpointer context, audiopcmhandle pcmHandle)
{
    assert(pcmHandle != nullptr);
    ++static_cast<TestAudioOperations*>(context)->closePcmCount;
}

/** @brief Opens the simulated persistent mixer handle. */
audiomixerhandle openMixer(contextpointer context, stringview deviceName)
{
    TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
    ++state.openMixerCount;
    state.mixerDevice = string(deviceName);
    return &state.mixerToken;
}

/** @brief Records one simulated mixer-element volume update. */
boolean setMixerVolume(contextpointer context, audiomixerhandle mixerHandle,
    stringview elementName, uint8 volumePercent)
{
    assert(mixerHandle != nullptr);
    TestAudioOperations& state = *static_cast<TestAudioOperations*>(context);
    ++state.setVolumeCount;
    state.mixerElement = string(elementName);
    state.volumePercent = volumePercent;
    return true;
}

/** @brief Records closure of the simulated persistent mixer handle. */
void closeMixer(contextpointer context, audiomixerhandle mixerHandle)
{
    assert(mixerHandle != nullptr);
    ++static_cast<TestAudioOperations*>(context)->closeMixerCount;
}

/** @brief Returns a complete injected ALSA operation table. */
XWalkAudioAlsaOperations testOperations()
{
    return {&openPcm, &configurePcm, &writePcm, &recoverPcm,
        &closePcm, &openMixer, &setMixerVolume, &closeMixer};
}

/** @brief Returns a valid signed sixteen-bit stereo stream configuration. */
XWalkAudioStreamConfiguration testConfiguration()
{
    return {44'100U, 2U, XWalkAudioSampleFormat::Signed16LittleEndian,
        256U, XHAL_RPI5CAR_AUDIO_DEFAULT_LATENCY_US};
}

/** @brief Verifies mixer ownership, negotiation, short writes, recovery, and explicit close. */
void testOwnershipAndWrites()
{
    TestAudioOperations state;
    {
        XWalkAudioAlsa audio(&state, testOperations(), "test-pcm", "test-mixer", "PCM");
        assert(state.openMixerCount == 1U);
        assert(state.mixerDevice == "test-mixer");

        const XWalkAudioStreamConfiguration configuration = testConfiguration();
        audiopcmhandle stream = audio.openStream(configuration);
        assert(stream != nullptr);
        assert(audio.openStreamCount() == 1U);
        assert(state.pcmDevice == "test-pcm");
        assert(state.configuration.sampleRateHz == 44'100U);
        assert(state.configuration.periodFrames == 256U);

        const size frameCount = 4U;
        const size bytesPerFrame = 4U;
        const bytevector silence(frameCount * bytesPerFrame, 0U);
        audio.writeFrames(stream, silence, frameCount);
        assert(state.recoverCount == 1U);
        assert(state.writeCount == 3U);
        assert(state.byteOffset == 8U);
        assert(state.requestedFrames == 2U);

        audio.setVolume(37U);
        assert(state.mixerElement == "PCM");
        assert(state.volumePercent == 37U);
        audio.closeStream(stream);
        assert(audio.openStreamCount() == 0U);
        assert(state.closePcmCount == 1U);
    }
    assert(state.closeMixerCount == 1U);
}

/** @brief Verifies that destruction closes every retained PCM handle before the mixer. */
void testDestructorCleanup()
{
    TestAudioOperations state;
    {
        XWalkAudioAlsa audio(&state, testOperations(), "pcm", "mixer", "PCM");
        static_cast<void>(audio.openStream(testConfiguration()));
        static_cast<void>(audio.openStream(testConfiguration()));
        assert(audio.openStreamCount() == 2U);
    }
    assert(state.closePcmCount == 2U);
    assert(state.closeMixerCount == 1U);
}

/** @brief Verifies the fixed eight-stream ownership limit. */
void testStreamLimit()
{
    TestAudioOperations state;
    XWalkAudioAlsa audio(&state, testOperations(), "pcm", "mixer", "PCM");
    for (size index = 0U; index < XHAL_RPI5CAR_AUDIO_MAXIMUM_STREAM_COUNT; ++index)
    {
        static_cast<void>(audio.openStream(testConfiguration()));
    }
    assert(audio.openStreamCount() == XHAL_RPI5CAR_AUDIO_MAXIMUM_STREAM_COUNT);
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(audio.openStream(testConfiguration()));
    });
}

/** @brief Verifies callback, configuration, ownership, payload, and recovery failures. */
void testValidationAndFailures()
{
    TestAudioOperations state;
    XWalkAudioAlsaOperations incompleteOperations = testOperations();
    incompleteOperations.writePcm = nullptr;
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkAudioAlsa audio(&state, incompleteOperations, "pcm", "mixer", "PCM");
        static_cast<void>(audio);
    });

    XWalkAudioAlsa audio(&state, testOperations(), "pcm", "mixer", "PCM");
    XWalkAudioStreamConfiguration invalidConfiguration = testConfiguration();
    invalidConfiguration.channelCount = 0U;
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(audio.openStream(invalidConfiguration));
    });

    audiopcmhandle stream = audio.openStream(testConfiguration());
    xwalk::hal::test::expectFailure([&]()
    {
        audio.writeFrames(stream, bytevector(3U, 0U), 1U);
    });
    xwalk::hal::test::expectFailure([&]()
    {
        const size excessiveFrames = static_cast<size>(testConfiguration().periodFrames) + 1U;
        audio.writeFrames(stream, bytevector(excessiveFrames * 4U, 0U), excessiveFrames);
    });
    xwalk::hal::test::expectFailure([&]()
    {
        audio.setVolume(101U);
    });
    audio.closeStream(stream);

    TestAudioOperations failedRecoveryState;
    failedRecoveryState.recoverSucceeds = false;
    XWalkAudioAlsa failedRecoveryAudio(&failedRecoveryState, testOperations(),
        "pcm", "mixer", "PCM");
    audiopcmhandle failedStream = failedRecoveryAudio.openStream(testConfiguration());
    xwalk::hal::test::expectFailure([&]()
    {
        failedRecoveryAudio.writeFrames(failedStream, bytevector(4U, 0U), 1U);
    });
    failedRecoveryAudio.closeStream(failedStream);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every injected ALSA software test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int main()
{
    testOwnershipAndWrites();
    testDestructorCleanup();
    testStreamLimit();
    testValidationAndFailures();
    return 0;
}
