/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerAlsaTest.cpp
 * @brief       Verifies Speaker decoding and ALSA callbacks without a device.
 *
 * @details
 * Covers built-in WAVE decoding, optional format routing, exact float32 bytes,
 * cancellation, task cleanup, bounds, malformed input, and stream failure.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker ALSA Adapter Host Test
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

#include "xHal_Rpi5CarSpeaker.h"
#include "xHal_Rpi5CarSpeakerAlsa.h"

#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <fstream>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains deterministic decoder and ALSA test operations.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records every observable adapter operation. */
struct TestBackend
{
    /** @brief Stable non-null simulated PCM handle. */
    XWalkHal::uint8 pcmToken{1U};
    /** @brief Stable non-null simulated mixer handle. */
    XWalkHal::uint8 mixerToken{2U};
    /** @brief Frame count returned by the optional decoder. */
    XWalkHal::size decodedFrameCount{4'096U};
    /** @brief Most recent decoder family. */
    XWalkHal::XWalkSpeakerAudioHandler handler{
        XWalkHal::XWalkSpeakerAudioHandler::SoundFile};
    /** @brief Number of decode operations. */
    XWalkHal::uint32 decodeCount{};
    /** @brief Number of configured streams. */
    XWalkHal::uint32 configureCount{};
    /** @brief Number of PCM write operations. */
    XWalkHal::uint32 writeCount{};
    /** @brief Number of PCM close operations. */
    XWalkHal::uint32 closeCount{};
    /** @brief Most recent configured mixer volume. */
    XWalkHal::uint8 volumePercent{};
    /** @brief Complete PCM bytes accepted across writes. */
    XWalkHal::bytevector writtenPcm{};
    /** @brief `true` when PCM writes must report a failure. */
    XWalkHal::boolean failWrite{};
    /** @brief `true` when decoded samples must have incomplete stereo alignment. */
    XWalkHal::boolean invalidAudio{};
    /** @brief `true` when decoded samples must exceed the configured bound. */
    XWalkHal::boolean excessiveAudio{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Stores one little-endian sixteen-bit fixture value. */
void writeUint16(XWalkHal::string& data, XWalkHal::size byteOffset,
    XWalkHal::uint16 value)
{
    data[byteOffset] = static_cast<char>(value & 0xFFU);
    data[byteOffset + 1U] = static_cast<char>((value >> 8U) & 0xFFU);
}

/** @brief Stores one little-endian thirty-two-bit fixture value. */
void writeUint32(XWalkHal::string& data, XWalkHal::size byteOffset,
    XWalkHal::uint32 value)
{
    for (XWalkHal::size byteIndex = 0U; byteIndex < 4U; ++byteIndex)
    {
        const XWalkHal::uint32 shiftBits = static_cast<XWalkHal::uint32>(byteIndex * 8U);
        data[byteOffset + byteIndex] = static_cast<char>((value >> shiftBits) & 0xFFU);
    }
}

/** @brief Writes four known mono samples in a valid PCM WAVE fixture. */
void writeWaveFixture(const XWalkHal::filesystempath& filePath)
{
    XWalkHal::string waveData(52U, '\0');
    waveData.replace(0U, 4U, "RIFF");
    writeUint32(waveData, 4U, 44U);
    waveData.replace(8U, 4U, "WAVE");
    waveData.replace(12U, 4U, "fmt ");
    writeUint32(waveData, 16U, 16U);
    writeUint16(waveData, 20U, 1U);
    writeUint16(waveData, 22U, 1U);
    writeUint32(waveData, 24U, 44'100U);
    writeUint32(waveData, 28U, 88'200U);
    writeUint16(waveData, 32U, 2U);
    writeUint16(waveData, 34U, 16U);
    waveData.replace(36U, 4U, "data");
    writeUint32(waveData, 40U, 8U);
    writeUint16(waveData, 44U, 0U);
    writeUint16(waveData, 46U, 16'384U);
    writeUint16(waveData, 48U, 32'768U);
    writeUint16(waveData, 50U, 32'767U);
    XWalkHal::outputfilestream file(filePath, XWalkHal::FILE_OPEN_WRITE_TRUNCATE);
    file << waveData;
    assert(file.good());
}

/** @brief Creates one empty optional-format fixture. */
void createEmptyFile(const XWalkHal::filesystempath& filePath)
{
    XWalkHal::outputfilestream file(filePath, XWalkHal::FILE_OPEN_WRITE_TRUNCATE);
    assert(file.is_open());
}

/** @brief Returns bounded normalized samples for optional format tests. */
XWalkHal::XWalkSpeakerAudioData decodeAudio(XWalkHal::contextpointer context,
    XWalkHal::stringview filePath, XWalkHal::XWalkSpeakerAudioHandler handler)
{
    static_cast<void>(filePath);
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.decodeCount;
    backend.handler = handler;
    if (backend.invalidAudio)
    {
        return {{0.0, 0.5, 1.0}, 44'100U, 2U};
    }
    if (backend.excessiveAudio)
    {
        const XWalkHal::size excessiveSampleCount =
            static_cast<XWalkHal::size>(XHAL_RPI5CAR_SPEAKER_MAXIMUM_DECODED_SAMPLE_COUNT) + 1U;
        return {XWalkHal::float64vector(excessiveSampleCount, 0.0), 44'100U, 1U};
    }
    return {XWalkHal::float64vector(backend.decodedFrameCount, 0.5), 44'100U, 1U};
}

/** @brief Opens one simulated PCM stream. */
XWalkHal::audiopcmhandle openPcm(XWalkHal::contextpointer context,
    XWalkHal::stringview deviceName)
{
    static_cast<void>(deviceName);
    return &static_cast<TestBackend*>(context)->pcmToken;
}

/** @brief Records and accepts one float32 PCM configuration. */
XWalkHal::boolean configurePcm(XWalkHal::contextpointer context,
    XWalkHal::audiopcmhandle pcmHandle,
    const XWalkHal::XWalkAudioStreamConfiguration& configuration)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(pcmHandle == &backend.pcmToken);
    assert(configuration.format == XWalkHal::XWalkAudioSampleFormat::Float32LittleEndian);
    assert(configuration.periodFrames == XHAL_RPI5CAR_SPEAKER_CHUNK_FRAME_COUNT);
    ++backend.configureCount;
    return true;
}

/** @brief Records one simulated PCM write or returns a configured failure. */
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
    assert(pcmData.size() == (frameCount * 4U));
    ++backend.writeCount;
    backend.writtenPcm.insert(backend.writtenPcm.end(), pcmData.begin(), pcmData.end());
    XWalkHal::common::sleepMilliseconds(1U);
    return static_cast<XWalkHal::int32>(frameCount);
}

/** @brief Rejects simulated PCM recovery. */
XWalkHal::boolean recoverPcm(XWalkHal::contextpointer context,
    XWalkHal::audiopcmhandle pcmHandle, XWalkHal::int32 errorValue)
{
    static_cast<void>(context);
    static_cast<void>(pcmHandle);
    static_cast<void>(errorValue);
    return false;
}

/** @brief Records one simulated PCM close. */
void closePcm(XWalkHal::contextpointer context, XWalkHal::audiopcmhandle pcmHandle)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(pcmHandle == &backend.pcmToken);
    ++backend.closeCount;
}

/** @brief Opens one simulated persistent mixer. */
XWalkHal::audiomixerhandle openMixer(XWalkHal::contextpointer context,
    XWalkHal::stringview deviceName)
{
    static_cast<void>(deviceName);
    return &static_cast<TestBackend*>(context)->mixerToken;
}

/** @brief Records one simulated mixer-volume operation. */
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

/** @brief Accepts simulated persistent mixer closure. */
void closeMixer(XWalkHal::contextpointer context, XWalkHal::audiomixerhandle mixerHandle)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(mixerHandle == &backend.mixerToken);
}

/** @brief Returns the complete simulated ALSA operation table. */
XWalkHal::XWalkAudioAlsaOperations audioOperations()
{
    return {&openPcm, &configurePcm, &writePcm, &recoverPcm,
        &closePcm, &openMixer, &setMixerVolume, &closeMixer};
}

/** @brief Verifies exact float32 bytes, volume, stream lifecycle, and task identifiers. */
void testCallbackConversion()
{
    TestBackend backend;
    XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
    const XWalkHal::XWalkSpeakerAlsaOperations operations{&decodeAudio};
    XWalkHal::XWalkSpeakerAlsa adapter(audio, &backend, operations, 15U);
    const XWalkHal::XWalkSpeakerCallbacks callbacks = adapter.callbacks();
    callbacks.enableOutput(&adapter);
    assert(backend.volumePercent == 15U);
    const XWalkHal::speakerstreamhandle stream = callbacks.openStream(&adapter, 44'100U, 1U);
    const XWalkHal::XWalkSpeakerAudioData audioData{{0.0, 0.5, -1.0, 1.0}, 44'100U, 1U};
    callbacks.writeStream(&adapter, stream, audioData, 0U, 4U);
    callbacks.closeStream(&adapter, stream);
    const XWalkHal::bytevector expectedPcm{
        0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x3FU,
        0x00U, 0x00U, 0x80U, 0xBFU,
        0x00U, 0x00U, 0x80U, 0x3FU};
    assert(backend.writtenPcm == expectedPcm);
    assert(callbacks.createTaskId(&adapter) != callbacks.createTaskId(&adapter));
}

/** @brief Verifies built-in WAVE decoding, short playback, and task cleanup. */
void testWavePlayback(const XWalkHal::filesystempath& wavePath)
{
    writeWaveFixture(wavePath);
    TestBackend backend;
    XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
    XWalkHal::XWalkSpeakerAlsa adapter(audio, 12U);
    XWalkHal::XWalkSpeaker speaker(&adapter, adapter.callbacks());
    static_cast<void>(speaker.play(wavePath.string()));
    XWalkHal::common::sleepMilliseconds(20U);
    assert(speaker.listTasks().empty());
    assert(backend.volumePercent == 12U);
    assert(backend.writtenPcm.size() == 16U);
    assert(backend.closeCount == 1U);
}

/** @brief Verifies optional decoder routing plus pause, resume, cancellation, and cleanup. */
void testOptionalFormats(const XWalkHal::filesystempath& oggPath,
    const XWalkHal::filesystempath& mp3Path)
{
    createEmptyFile(oggPath);
    createEmptyFile(mp3Path);
    TestBackend backend;
    backend.decodedFrameCount = 65'536U;
    XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
    const XWalkHal::XWalkSpeakerAlsaOperations operations{&decodeAudio};
    XWalkHal::XWalkSpeakerAlsa adapter(audio, &backend, operations, 10U);
    XWalkHal::XWalkSpeaker speaker(&adapter, adapter.callbacks());
    const XWalkHal::string oggTask = speaker.play(oggPath.string());
    assert(backend.handler == XWalkHal::XWalkSpeakerAudioHandler::SoundFile);
    XWalkHal::common::sleepMilliseconds(2U);
    speaker.pause(oggTask);
    speaker.resume(oggTask);
    assert(speaker.stop(oggTask));

    const XWalkHal::string mp3Task = speaker.play(mp3Path.string());
    assert(backend.handler == XWalkHal::XWalkSpeakerAudioHandler::Librosa);
    assert(speaker.stop(mp3Task));
    assert(speaker.listTasks().empty());
    assert(backend.closeCount == 2U);
}

/** @brief Verifies malformed input, invalid setup, decoded bounds, and stream failure. */
void testValidation(const XWalkHal::filesystempath& wavePath,
    const XWalkHal::filesystempath& oggPath)
{
    TestBackend backend;
    XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
    const XWalkHal::XWalkSpeakerAlsaOperations missingOperations{};
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkHal::XWalkSpeakerAlsa adapter(audio, &backend, missingOperations, 10U);
    });
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkHal::XWalkSpeakerAlsa adapter(audio, 101U);
    });

    const XWalkHal::XWalkSpeakerAlsaOperations decoderOperations{&decodeAudio};
    XWalkHal::XWalkSpeakerAlsa injectedAdapter(audio, &backend, decoderOperations, 10U);
    const XWalkHal::XWalkSpeakerCallbacks callbacks = injectedAdapter.callbacks();
    backend.invalidAudio = true;
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(callbacks.decodeAudio(&injectedAdapter, "invalid.wav",
            XWalkHal::XWalkSpeakerAudioHandler::SoundFile));
    });
    backend.invalidAudio = false;
    backend.excessiveAudio = true;
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(callbacks.decodeAudio(&injectedAdapter, "excessive.wav",
            XWalkHal::XWalkSpeakerAudioHandler::SoundFile));
    });
    backend.excessiveAudio = false;

    createEmptyFile(wavePath);
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkHal::XWalkSpeakerAlsa adapter(audio, 10U);
        XWalkHal::XWalkSpeaker speaker(&adapter, adapter.callbacks());
        static_cast<void>(speaker.play(wavePath.string()));
    });

    xwalk::hal::test::expectFailure([&]()
    {
        TestBackend failingBackend;
        failingBackend.failWrite = true;
        XWalkHal::XWalkAudioAlsa failingAudio(&failingBackend, audioOperations(),
            "pcm", "mixer", "PCM");
        const XWalkHal::XWalkSpeakerAlsaOperations operations{&decodeAudio};
        XWalkHal::XWalkSpeakerAlsa adapter(failingAudio, &failingBackend, operations, 10U);
        XWalkHal::XWalkSpeaker speaker(&adapter, adapter.callbacks());
        static_cast<void>(speaker.play(oggPath.string()));
        XWalkHal::common::sleepMilliseconds(20U);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side Speaker ALSA adapter tests.
 *
 * @param[in] argumentCount
 * Program name followed by WAVE, OGG, and MP3 fixture paths.
 *
 * @param[in] argumentValues
 * Program arguments containing three module-local fixture paths.
 *
 * @return
 * Zero after every assertion passes.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer argumentValues[])
{
    assert(argumentCount == 4);
    const XWalkHal::filesystempath wavePath(argumentValues[1]);
    const XWalkHal::filesystempath oggPath(argumentValues[2]);
    const XWalkHal::filesystempath mp3Path(argumentValues[3]);
    testCallbackConversion();
    testWavePlayback(wavePath);
    testOptionalFormats(oggPath, mp3Path);
    testValidation(wavePath, oggPath);
    static_cast<void>(XWalkHal::removeFilesystemEntry(wavePath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(oggPath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(mp3Path));
    return 0;
}
