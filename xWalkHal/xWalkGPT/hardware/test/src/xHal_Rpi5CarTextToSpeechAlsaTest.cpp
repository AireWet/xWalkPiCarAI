/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechAlsaTest.cpp
 * @brief       Verifies synthesis and ALSA playback without audio hardware.
 *
 * @details
 * Covers PCM validation, bounded chunking, conservative volume, empty output,
 * provider failure, stream failure, and non-owning lifecycle behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Text-to-Speech ALSA Host Test
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

#include "xHal_Rpi5CarTextToSpeechAlsa.h"

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains deterministic synthesis and ALSA operations.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records observable provider and shared-audio operations. */
struct TestBackend
{
    /** @brief Stable non-null simulated PCM handle. */
    XWalkHal::uint8 pcmToken{1U};
    /** @brief Stable non-null simulated mixer handle. */
    XWalkHal::uint8 mixerToken{2U};
    /** @brief Provider PCM result returned by value. */
    XWalkHal::XWalkTextToSpeechPcmData audioData{};
    /** @brief Owned copy of the latest synthesis text. */
    XWalkHal::string text{};
    /** @brief Frame count accepted by each shared-audio write. */
    XWalkHal::uint32vector frameCounts{};
    /** @brief Complete PCM bytes accepted across all writes. */
    XWalkHal::bytevector writtenPcm{};
    /** @brief Number of synthesis calls. */
    XWalkHal::uint32 synthesisCount{};
    /** @brief Number of opened PCM handles. */
    XWalkHal::uint32 openCount{};
    /** @brief Number of closed PCM handles. */
    XWalkHal::uint32 closeCount{};
    /** @brief Most recent configured mixer volume. */
    XWalkHal::uint8 volumePercent{};
    /** @brief Makes synthesis report a provider failure when true. */
    XWalkHal::boolean failSynthesis{};
    /** @brief Makes PCM writes report an unrecoverable failure when true. */
    XWalkHal::boolean failWrite{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Returns configured provider PCM while retaining the requested text.
 *
 * @param[in,out] context Non-null test backend context.
 * @param[in] text Text view retained as an owned test copy.
 * @return Configured PCM result.
 * @throws std::runtime_error When provider failure is enabled.
 */
XWalkHal::XWalkTextToSpeechPcmData synthesize(XWalkHal::contextpointer context,
    XWalkHal::stringview text)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.synthesisCount;
    backend.text = text;
    if (backend.failSynthesis)
    {
        XHAL_THROW_RUNTIME_ERROR("Simulated synthesis failure");
    }
    return backend.audioData;
}

/**
 * @brief Opens one simulated PCM handle.
 *
 * @param[in,out] context Non-null test backend context.
 * @param[in] deviceName Expected simulated PCM device name.
 * @return Stable non-null simulated PCM handle.
 */
XWalkHal::audiopcmhandle openPcm(XWalkHal::contextpointer context,
    XWalkHal::stringview deviceName)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(deviceName == "pcm");
    ++backend.openCount;
    return &backend.pcmToken;
}

/**
 * @brief Validates and accepts one signed sixteen-bit stream configuration.
 *
 * @param[in,out] context Non-null test backend context.
 * @param[in,out] pcmHandle Stable simulated PCM handle.
 * @param[in] configuration Configuration supplied by the adapter.
 * @return Always `true` after assertions pass.
 */
XWalkHal::boolean configurePcm(XWalkHal::contextpointer context,
    XWalkHal::audiopcmhandle pcmHandle,
    const XWalkHal::XWalkAudioStreamConfiguration& configuration)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(pcmHandle == &backend.pcmToken);
    assert(configuration.sampleRateHz == backend.audioData.sampleRateHz);
    assert(configuration.channelCount == backend.audioData.channelCount);
    assert(configuration.format == XWalkHal::XWalkAudioSampleFormat::Signed16LittleEndian);
    assert(configuration.periodFrames == XHAL_RPI5CAR_TEXT_TO_SPEECH_PERIOD_FRAMES);
    return true;
}

/**
 * @brief Records one bounded PCM write or reports a configured failure.
 *
 * @param[in,out] context Non-null test backend context.
 * @param[in,out] pcmHandle Stable simulated PCM handle.
 * @param[in] pcmData Complete period PCM supplied by shared audio.
 * @param[in] byteOffset Byte offset within `pcmData`.
 * @param[in] frameCount Complete frames supplied by this operation.
 * @return Written frame count, or a negative simulated error.
 */
XWalkHal::int32 writePcm(XWalkHal::contextpointer context,
    XWalkHal::audiopcmhandle pcmHandle, const XWalkHal::bytevector& pcmData,
    XWalkHal::size byteOffset, XWalkHal::size frameCount)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(pcmHandle == &backend.pcmToken);
    assert(byteOffset == 0U);
    if (backend.failWrite)
    {
        return -5;
    }
    backend.frameCounts.push_back(static_cast<XWalkHal::uint32>(frameCount));
    backend.writtenPcm.insert(backend.writtenPcm.end(), pcmData.begin(), pcmData.end());
    return static_cast<XWalkHal::int32>(frameCount);
}

/**
 * @brief Rejects recovery from a simulated PCM failure.
 *
 * @param[in,out] context Unused test backend context.
 * @param[in,out] pcmHandle Unused simulated PCM handle.
 * @param[in] errorValue Unused negative write result.
 * @return Always `false`.
 */
XWalkHal::boolean recoverPcm(XWalkHal::contextpointer context,
    XWalkHal::audiopcmhandle pcmHandle, XWalkHal::int32 errorValue)
{
    static_cast<void>(context);
    static_cast<void>(pcmHandle);
    static_cast<void>(errorValue);
    return false;
}

/**
 * @brief Records closure of one simulated PCM handle.
 *
 * @param[in,out] context Non-null test backend context.
 * @param[in,out] pcmHandle Stable simulated PCM handle.
 */
void closePcm(XWalkHal::contextpointer context, XWalkHal::audiopcmhandle pcmHandle)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(pcmHandle == &backend.pcmToken);
    ++backend.closeCount;
}

/**
 * @brief Opens one simulated persistent mixer.
 *
 * @param[in,out] context Non-null test backend context.
 * @param[in] deviceName Expected simulated mixer device name.
 * @return Stable non-null simulated mixer handle.
 */
XWalkHal::audiomixerhandle openMixer(XWalkHal::contextpointer context,
    XWalkHal::stringview deviceName)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(deviceName == "mixer");
    return &backend.mixerToken;
}

/**
 * @brief Records one simulated mixer-volume operation.
 *
 * @param[in,out] context Non-null test backend context.
 * @param[in,out] mixerHandle Stable simulated mixer handle.
 * @param[in] elementName Expected simulated mixer element.
 * @param[in] volumePercent Volume supplied by the adapter.
 * @return Always `true` after assertions pass.
 */
XWalkHal::boolean setMixerVolume(XWalkHal::contextpointer context,
    XWalkHal::audiomixerhandle mixerHandle, XWalkHal::stringview elementName,
    XWalkHal::uint8 volumePercent)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(mixerHandle == &backend.mixerToken);
    assert(elementName == "PCM");
    backend.volumePercent = volumePercent;
    return true;
}

/**
 * @brief Accepts closure of the simulated persistent mixer.
 *
 * @param[in,out] context Non-null test backend context.
 * @param[in,out] mixerHandle Stable simulated mixer handle.
 */
void closeMixer(XWalkHal::contextpointer context, XWalkHal::audiomixerhandle mixerHandle)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(mixerHandle == &backend.mixerToken);
}

/**
 * @brief Returns the complete simulated ALSA operation table.
 * @return Eight non-null deterministic ALSA operations.
 */
XWalkHal::XWalkAudioAlsaOperations audioOperations()
{
    return {&openPcm, &configurePcm, &writePcm, &recoverPcm,
        &closePcm, &openMixer, &setMixerVolume, &closeMixer};
}

/**
 * @brief Verifies exact bounded chunks, PCM bytes, text, volume, and cleanup.
 */
void testSynthesisPlayback()
{
    TestBackend backend;
    backend.audioData = {XWalkHal::bytevector(5'000U, 0x24U), 16'000U, 1U};
    XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
    const XWalkHal::XWalkTextToSpeechAlsaOperations operations{&synthesize};
    XWalkHal::XWalkTextToSpeechAlsa adapter(audio, &backend, operations, 15U);
    adapter.callback()(&adapter, "bounded speech");
    assert(backend.text == "bounded speech");
    assert(backend.synthesisCount == 1U);
    assert(backend.volumePercent == 15U);
    assert(backend.frameCounts == XWalkHal::uint32vector({1'024U, 1'024U, 452U}));
    assert(backend.writtenPcm == backend.audioData.pcmData);
    assert(backend.openCount == 1U && backend.closeCount == 1U);
}

/**
 * @brief Verifies empty provider output performs no ALSA playback.
 */
void testEmptyOutput()
{
    TestBackend backend;
    XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
    const XWalkHal::XWalkTextToSpeechAlsaOperations operations{&synthesize};
    XWalkHal::XWalkTextToSpeechAlsa adapter(audio, &backend, operations, 15U);
    adapter.callback()(&adapter, "");
    assert(backend.synthesisCount == 1U);
    assert(backend.text.empty());
    assert(backend.openCount == 0U && backend.closeCount == 0U);
}

/**
 * @brief Verifies construction, PCM, provider, and stream failures.
 */
void testValidation()
{
    TestBackend backend;
    XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
    xwalk::hal::test::expectFailure([&]()
    {
        const XWalkHal::XWalkTextToSpeechAlsaOperations missing{};
        XWalkHal::XWalkTextToSpeechAlsa adapter(audio, &backend, missing, 15U);
    });
    xwalk::hal::test::expectFailure([&]()
    {
        const XWalkHal::XWalkTextToSpeechAlsaOperations operations{&synthesize};
        XWalkHal::XWalkTextToSpeechAlsa adapter(audio, &backend, operations, 101U);
    });

    const XWalkHal::XWalkTextToSpeechAlsaOperations operations{&synthesize};
    XWalkHal::XWalkTextToSpeechAlsa adapter(audio, &backend, operations, 15U);
    backend.audioData = {{0U, 1U, 2U}, 16'000U, 1U};
    xwalk::hal::test::expectFailure([&]()
    {
        adapter.callback()(&adapter, "incomplete");
    });
    backend.audioData = {XWalkHal::bytevector(2U, 0U), 0U, 1U};
    xwalk::hal::test::expectFailure([&]()
    {
        adapter.callback()(&adapter, "metadata");
    });
    backend.audioData = {XWalkHal::bytevector(
        XHAL_RPI5CAR_TEXT_TO_SPEECH_MAXIMUM_PCM_BYTES + 1U, 0U), 16'000U, 1U};
    xwalk::hal::test::expectFailure([&]()
    {
        adapter.callback()(&adapter, "excessive");
    });
    backend.audioData = {XWalkHal::bytevector(2U, 0U), 16'000U, 1U};
    backend.failSynthesis = true;
    xwalk::hal::test::expectFailure([&]()
    {
        adapter.callback()(&adapter, "provider failure");
    });
    backend.failSynthesis = false;
    backend.failWrite = true;
    xwalk::hal::test::expectFailure([&]()
    {
        adapter.callback()(&adapter, "stream failure");
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all text-to-speech ALSA host scenarios.
 * @return Zero after every assertion passes.
 */
XWalkHal::int32 main()
{
    testSynthesisPlayback();
    testEmptyOutput();
    testValidation();
    return 0;
}
