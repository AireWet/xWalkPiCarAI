/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsaTest.cpp
 * @brief       Verifies the music ALSA adapter without an audio device.
 *
 * @details
 * Injects decoder and ALSA operation tables to verify callback routing, PCM
 * byte counts, volume, loops, background playback, and transport controls.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic ALSA Adapter Host Test
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

#include "xHal_Rpi5CarMusicAlsa.h"
#include "xHal_Rpi5CarMusic.h"

#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include "xHal_Rpi5CarTrace.h"
#include <fstream>
#include "xHal_Rpi5CarMusicAlsaTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TestBackend = ::xwalk::source_types::xhal_rpi5carmusicalsatest::TestBackend;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains simulated decoder and ALSA state private to this test.
 */
namespace
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /******************************************************************************
     * Private function definitions
     ******************************************************************************/

    /** @brief Stores one little-endian sixteen-bit fixture value. */
    void writeUint16(XWalkHal::string& data, XWalkHal::size byteOffset, XWalkHal::uint16 value)
    {
        data[byteOffset] = static_cast<char>(value & 0xFFU);
        data[byteOffset + 1U] = static_cast<char>((value >> 8U) & 0xFFU);
    }

    /** @brief Stores one little-endian thirty-two-bit fixture value. */
    void writeUint32(XWalkHal::string& data, XWalkHal::size byteOffset, XWalkHal::uint32 value)
    {
        for (XWalkHal::size byteIndex = 0U; byteIndex < 4U; ++byteIndex)
        {
            const XWalkHal::uint32 shiftBits = static_cast<XWalkHal::uint32>(byteIndex * 8U);
            data[byteOffset + byteIndex] = static_cast<char>((value >> shiftBits) & 0xFFU);
        }
    }

    /** @brief Writes a two-frame mono sixteen-bit PCM WAVE decoder fixture. */
    void writeWaveFixture(XWalkHal::stringview filename)
    {
        XWalkHal::string waveData(48U, '\0');
        waveData.replace(0U, 4U, "RIFF");
        writeUint32(waveData, 4U, 40U);
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
        writeUint32(waveData, 40U, 4U);
        waveData[44U] = static_cast<char>(0x01U);
        waveData[45U] = static_cast<char>(0x02U);
        waveData[46U] = static_cast<char>(0x03U);
        waveData[47U] = static_cast<char>(0x04U);

        XWalkHal::outputfilestream file(XWalkHal::filesystempath{XWalkHal::string(filename)},
                                        XWalkHal::FILE_OPEN_WRITE_TRUNCATE);
        file << waveData;
        const hal::boolean fixtureWriteFailed = static_cast<hal::boolean>(!file.good());
        if (fixtureWriteFailed)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Music ALSA test could not write its WAVE fixture");
        }
    }

    /** @brief Returns one deterministic mono PCM fixture. */
    XWalkHal::XWalkMusicAlsaAudioData decodeAudio(XWalkHal::contextpointer context, XWalkHal::stringview filename)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        ++backend.decodeCount;
        const XWalkHal::size frameCount = (filename == "long.wav") ? 32'768U : 2'048U;
        return {XWalkHal::bytevector(frameCount * 2U, 0x11U), 44'100U, 1U};
    }

    /** @brief Returns the simulated PCM handle. */
    XWalkHal::audiopcmhandle openPcm(XWalkHal::contextpointer context, XWalkHal::stringview deviceName)
    {
        static_cast<void>(deviceName);
        TestBackend& backend = *static_cast<TestBackend*>(context);
        backend.volumeObservedAtOpen = backend.volumeSet;
        return &backend.pcmToken;
    }

    /** @brief Records one valid stream configuration. */
    XWalkHal::boolean configurePcm(XWalkHal::contextpointer context,
                                   XWalkHal::audiopcmhandle pcmHandle,
                                   const XWalkHal::XWalkAudioStreamConfiguration& configuration)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        assert(pcmHandle == &backend.pcmToken);
        assert(configuration.format == XWalkHal::XWalkAudioSampleFormat::Signed16LittleEndian);
        ++backend.configureCount;
        return true;
    }

    /** @brief Records one complete simulated PCM write. */
    XWalkHal::int32 writePcm(XWalkHal::contextpointer context,
                             XWalkHal::audiopcmhandle pcmHandle,
                             const XWalkHal::bytevector& pcmData,
                             XWalkHal::size byteOffset,
                             XWalkHal::size frameCount)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        assert(pcmHandle == &backend.pcmToken);
        assert(byteOffset == 0U);
        assert(pcmData.size() == (frameCount * 2U));
        ++backend.writeCount;
        backend.writtenBytes += pcmData.size();
        return static_cast<XWalkHal::int32>(frameCount);
    }

    /** @brief Rejects recovery because simulated writes never fail. */
    XWalkHal::boolean
    recoverPcm(XWalkHal::contextpointer context, XWalkHal::audiopcmhandle pcmHandle, XWalkHal::int32 errorValue)
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

    /** @brief Returns the simulated persistent mixer handle. */
    XWalkHal::audiomixerhandle openMixer(XWalkHal::contextpointer context, XWalkHal::stringview deviceName)
    {
        static_cast<void>(deviceName);
        return &static_cast<TestBackend*>(context)->mixerToken;
    }

    /** @brief Records one simulated mixer volume. */
    XWalkHal::boolean setMixerVolume(XWalkHal::contextpointer context,
                                     XWalkHal::audiomixerhandle mixerHandle,
                                     XWalkHal::stringview elementName,
                                     XWalkHal::uint8 volumePercent)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        assert(mixerHandle == &backend.mixerToken);
        assert(elementName == "PCM");
        backend.volumePercent = volumePercent;
        backend.volumeSet = true;
        return true;
    }

    /** @brief Accepts simulated mixer closure. */
    void closeMixer(XWalkHal::contextpointer context, XWalkHal::audiomixerhandle mixerHandle)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        assert(mixerHandle == &backend.mixerToken);
    }

    /** @brief Returns the complete simulated ALSA operation table. */
    XWalkHal::XWalkAudioAlsaOperations audioOperations()
    {
        return {&openPcm, &configurePcm, &writePcm, &recoverPcm, &closePcm, &openMixer, &setMixerVolume, &closeMixer};
    }

    /** @brief Verifies synchronous callbacks, volume conversion, and PCM byte
     * counts. */
    void testSynchronousCallbacks()
    {
        TestBackend backend;
        XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
        const XWalkHal::XWalkMusicAlsaOperations decoderOperations{&decodeAudio};
        XWalkHal::XWalkMusicAlsa adapter(audio, &backend, decoderOperations);
        const XWalkHal::XWalkMusicCallbacks callbacks = adapter.callbacks();
        XWalkHal::XWalkMusic music(&adapter, callbacks);

        music.soundPlay("effect.wav", 25.0);
        assert(backend.volumePercent == 25U);
        assert(backend.writtenBytes == 4'096U);
        assert(music.soundLength("effect.wav") == 0.05);
        const XWalkHal::size writtenBeforeTone = backend.writtenBytes;
        music.playToneFor(440.0, 0.001);
        assert((backend.writtenBytes - writtenBeforeTone) == 88U);
        music.musicSetVolume(63.0);
        assert(backend.volumePercent == 63U);
        assert(backend.configureCount == 2U);
        assert(backend.closeCount == 2U);
    }

    /** @brief Verifies streamed loops, pause, resume, stop, and background
     * replacement. */
    void testWorkerCallbacks()
    {
        TestBackend backend;
        XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
        const XWalkHal::XWalkMusicAlsaOperations decoderOperations{&decodeAudio};
        XWalkHal::XWalkMusicAlsa adapter(audio, &backend, decoderOperations);
        XWalkHal::XWalkMusic music(&adapter, adapter.callbacks());

        const XWalkHal::size writtenBeforeMusic = backend.writtenBytes;
        music.musicPlay("song.wav", 1, 0.0, 30.0);
        xwalk::hal::common::sleepMilliseconds(20U);
        music.musicPause();
        music.musicResume();
        music.musicStop();
        assert((backend.writtenBytes - writtenBeforeMusic) == 8'192U);
        assert(backend.volumeObservedAtOpen);

        music.soundPlayBackground("long.wav", 10.0);
        music.soundPlayBackground("effect.wav", 20.0);
        assert(backend.volumePercent == 20U);
    }

    /** @brief Verifies adapter callback and decoded-data validation failures. */
    void testValidation()
    {
        TestBackend backend;
        XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
        const XWalkHal::XWalkMusicAlsaOperations missingOperations{};
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkMusicAlsa adapter(audio, &backend, missingOperations);
            });

        const XWalkHal::XWalkMusicAlsaOperations decoderOperations{&decodeAudio};
        XWalkHal::XWalkMusicAlsa adapter(audio, &backend, decoderOperations);
        const XWalkHal::XWalkMusicCallbacks callbacks = adapter.callbacks();
        xwalk::hal::test::expectFailure(
            [&]()
            {
                callbacks.setMusicVolume(&adapter, 1.01);
            });
    }

    /** @brief Verifies the production RIFF/WAVE decoder and its exact PCM bytes. */
    void testWaveDecoder(XWalkHal::stringview filename)
    {
        writeWaveFixture(filename);
        TestBackend backend;
        XWalkHal::XWalkAudioAlsa audio(&backend, audioOperations(), "pcm", "mixer", "PCM");
        XWalkHal::XWalkMusicAlsa adapter(audio);
        const XWalkHal::XWalkMusicCallbacks callbacks = adapter.callbacks();
        const XWalkHal::float64 durationSeconds = callbacks.getSoundLength(&adapter, filename);
        assert(XHAL_ABSOLUTE_VALUE(durationSeconds - (2.0 / 44'100.0)) < 0.000001);
        callbacks.playSound(&adapter, filename, {});
        assert(backend.writtenBytes == 4U);
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side music ALSA adapter tests.
 *
 * @param[in] argumentCount
 * Program name followed by one generated WAVE fixture path.
 *
 * @param[in] argumentValues
 * Program arguments containing the fixture path at index one.
 *
 * @return
 * Zero after every assertion passes.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, char* argumentValues[])
{
    assert(argumentCount == 2);
    testSynchronousCallbacks();
    testWorkerCallbacks();
    testValidation();
    testWaveDecoder(argumentValues[1]);
    return 0;
}
