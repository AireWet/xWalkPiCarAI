/******************************************************************************
 * @file        xHal_Rpi5CarMusicTest.cpp
 * @brief       Verifies music behavior using an in-memory audio backend.
 *
 * @details
 * Checks construction, timing, keys, note frequencies, volume conversion,
 * playback routing, tone compatibility, and public validation failures.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic Host Test
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarMusic.h"

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test state and callbacks private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records every operation received by the simulated audio backend. */
struct TestBackend
{
    /** @brief Number of output-enable operations. */
    XWalkHal::uint32 enableCount{};
    /** @brief Number of synchronous sound-effect operations. */
    XWalkHal::uint32 soundCount{};
    /** @brief Number of background sound-effect operations. */
    XWalkHal::uint32 backgroundSoundCount{};
    /** @brief Number of streamed-music start operations. */
    XWalkHal::uint32 musicCount{};
    /** @brief Number of streamed-music volume operations. */
    XWalkHal::uint32 volumeCount{};
    /** @brief Number of streamed-music stop operations. */
    XWalkHal::uint32 stopCount{};
    /** @brief Number of streamed-music pause operations. */
    XWalkHal::uint32 pauseCount{};
    /** @brief Number of streamed-music resume operations. */
    XWalkHal::uint32 resumeCount{};
    /** @brief Number of generated-tone write operations. */
    XWalkHal::uint32 toneCount{};
    /** @brief Most recent filename copied during a callback. */
    XWalkHal::string filename{};
    /** @brief Most recent optional normalized sound-effect volume. */
    XWalkHal::optionalfloat64 soundVolume{};
    /** @brief Most recent normalized streamed-music volume. */
    XWalkHal::float64 musicVolume{};
    /** @brief Most recent Python-compatible loop argument. */
    XWalkHal::int32 loops{};
    /** @brief Most recent streamed-music start offset in seconds. */
    XWalkHal::float64 startSeconds{};
    /** @brief Sound duration returned by the simulated backend, in seconds. */
    XWalkHal::float64 soundDurationSeconds{1.234};
    /** @brief Most recent generated PCM payload. */
    XWalkHal::bytevector pcmData{};
    /** @brief Most recent generated PCM sample rate in Hertz. */
    XWalkHal::uint32 sampleRateHz{};
    /** @brief Most recent generated PCM channel count. */
    XWalkHal::uint8 channelCount{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Records one simulated output-enable operation. */
void enableOutput(XWalkHal::contextpointer context)
{
    ++static_cast<TestBackend*>(context)->enableCount;
}

/** @brief Records one synchronous simulated sound-effect operation. */
void playSound(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::optionalfloat64 normalizedVolume)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.soundCount;
    backend.filename = XWalkHal::string(filename);
    backend.soundVolume = normalizedVolume;
}

/** @brief Records one background simulated sound-effect operation. */
void playSoundBackground(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::optionalfloat64 normalizedVolume)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.backgroundSoundCount;
    backend.filename = XWalkHal::string(filename);
    backend.soundVolume = normalizedVolume;
}

/** @brief Records one simulated streamed-music start operation. */
void playMusic(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::int32 loops, XWalkHal::float64 startSeconds)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.musicCount;
    backend.filename = XWalkHal::string(filename);
    backend.loops = loops;
    backend.startSeconds = startSeconds;
}

/** @brief Records one simulated streamed-music volume operation. */
void setMusicVolume(XWalkHal::contextpointer context, XWalkHal::float64 normalizedVolume)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.volumeCount;
    backend.musicVolume = normalizedVolume;
}

/** @brief Records one simulated streamed-music stop operation. */
void stopMusic(XWalkHal::contextpointer context)
{
    ++static_cast<TestBackend*>(context)->stopCount;
}

/** @brief Records one simulated streamed-music pause operation. */
void pauseMusic(XWalkHal::contextpointer context)
{
    ++static_cast<TestBackend*>(context)->pauseCount;
}

/** @brief Records one simulated streamed-music resume operation. */
void resumeMusic(XWalkHal::contextpointer context)
{
    ++static_cast<TestBackend*>(context)->resumeCount;
}

/** @brief Returns the configured simulated sound duration in seconds. */
XWalkHal::float64 getSoundLength(XWalkHal::contextpointer context,
    XWalkHal::stringview filename)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    backend.filename = XWalkHal::string(filename);
    return backend.soundDurationSeconds;
}

/** @brief Records generated PCM data and its format. */
void playTone(XWalkHal::contextpointer context, const XWalkHal::bytevector& pcmData,
    XWalkHal::uint32 sampleRateHz, XWalkHal::uint8 channelCount)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.toneCount;
    backend.pcmData = pcmData;
    backend.sampleRateHz = sampleRateHz;
    backend.channelCount = channelCount;
}

/** @brief Returns the complete callback table used by valid test scenarios. */
XWalkHal::XWalkMusicCallbacks musicCallbacks()
{
    return {&enableOutput, &playSound, &playSoundBackground, &playMusic,
        &setMusicVolume, &stopMusic, &pauseMusic, &resumeMusic,
        &getSoundLength, &playTone};
}

/** @brief Verifies default state, time signatures, tempo, and beat conversion. */
void testTiming()
{
    TestBackend backend;
    const XWalkHal::XWalkMusicCallbacks callbacks = musicCallbacks();
    XWalkHal::XWalkMusic music(&backend, callbacks);
    assert(backend.enableCount == 1U);
    assert(music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_TOP_INDEX] == 4U);
    assert(music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_BOTTOM_INDEX] == 4U);
    assert(music.tempo()[XHAL_RPI5CAR_MUSIC_TEMPO_BPM_INDEX] == 120.0);
    assert(music.tempo()[XHAL_RPI5CAR_MUSIC_TEMPO_NOTE_VALUE_INDEX] ==
        XHAL_RPI5CAR_MUSIC_QUARTER_NOTE);
    assert(music.beatDurationSeconds(XHAL_RPI5CAR_MUSIC_QUARTER_NOTE) == 0.5);

    music.setTimeSignature(3U, 8U);
    assert(music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_TOP_INDEX] == 3U);
    assert(music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_BOTTOM_INDEX] == 8U);
    music.setTimeSignature(6U);
    assert(music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_TOP_INDEX] == 6U);
    assert(music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_BOTTOM_INDEX] == 6U);
    music.setTempo(60.0, XHAL_RPI5CAR_MUSIC_HALF_NOTE);
    assert(music.beatDurationSeconds(XHAL_RPI5CAR_MUSIC_WHOLE_NOTE) == 2.0);
}

/** @brief Verifies numeric and named note conversion with key displacement. */
void testNotes()
{
    TestBackend backend;
    const XWalkHal::XWalkMusicCallbacks callbacks = musicCallbacks();
    XWalkHal::XWalkMusic music(&backend, callbacks);
    assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz(69) - 440.0) < 0.000001);
    assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz("A4") - 440.0) < 0.000001);
    music.setKeySignature(XHAL_RPI5CAR_MUSIC_KEY_G_MAJOR);
    assert(music.keySignature() == XHAL_RPI5CAR_MUSIC_KEY_G_MAJOR);
    assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz("A4") - 466.1637615) < 0.0001);
    assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz("A4", true) - 440.0) < 0.000001);
    music.setKeySignature("bbb");
    assert(music.keySignature() == -3);
    music.setKeySignature(0);
    assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz("C8") - 4'186.009045) < 0.001);
}

/** @brief Verifies sound, music, control, and duration callback routing. */
void testPlayback()
{
    TestBackend backend;
    const XWalkHal::XWalkMusicCallbacks callbacks = musicCallbacks();
    XWalkHal::XWalkMusic music(&backend, callbacks);
    music.soundPlay("effect.wav", 51.4);
    assert(backend.soundCount == 1U);
    assert(backend.filename == "effect.wav");
    assert(backend.soundVolume.has_value());
    assert(*backend.soundVolume == 0.51);

    music.soundPlayBackground("background.wav");
    assert(backend.backgroundSoundCount == 1U);
    assert(!backend.soundVolume.has_value());
    music.musicPlay("song.ogg", 2, 1.5, 75.0);
    assert(backend.volumeCount == 1U);
    assert(backend.musicVolume == 0.75);
    assert(backend.musicCount == 1U);
    assert(backend.loops == 2);
    assert(backend.startSeconds == 1.5);
    music.musicStop();
    music.musicPause();
    music.musicResume();
    music.musicUnpause();
    assert(backend.stopCount == 1U);
    assert(backend.pauseCount == 1U);
    assert(backend.resumeCount == 2U);
    assert(music.soundLength("effect.wav") == 1.23);
}

/** @brief Verifies Python-compatible tone size, silence, and backend format. */
void testToneGeneration()
{
    TestBackend backend;
    const XWalkHal::XWalkMusicCallbacks callbacks = musicCallbacks();
    XWalkHal::XWalkMusic music(&backend, callbacks);
    const XWalkHal::bytevector pcmData = music.getToneData(440.0, 0.001);
    assert(pcmData.size() == 88U);
    assert(pcmData[0U] == 0U);
    assert(pcmData[1U] == 0U);
    assert(pcmData[2U] != 0U);
    music.playToneFor(440.0, 0.001);
    assert(backend.toneCount == 1U);
    assert(backend.pcmData == pcmData);
    assert(backend.sampleRateHz == 44'100U);
    assert(backend.channelCount == 1U);
}

/** @brief Verifies callback and public numeric validation failures. */
void testValidation()
{
    TestBackend backend;
    XWalkHal::XWalkMusicCallbacks callbacks = musicCallbacks();
    callbacks.playTone = nullptr;
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkHal::XWalkMusic music(&backend, callbacks);
    });

    callbacks = musicCallbacks();
    XWalkHal::XWalkMusic music(&backend, callbacks);
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(music.noteFrequencyHz("H4"));
    });

    xwalk::hal::test::expectFailure([&]()
    {
        music.musicSetVolume(101.0);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side music tests.
 *
 * @return
 * Zero after every assertion passes.
 */
XWalkHal::int32 main()
{
    testTiming();
    testNotes();
    testPlayback();
    testToneGeneration();
    testValidation();
    return 0;
}
