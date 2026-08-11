/******************************************************************************
 * @file        xHal_Rpi5CarToneSequenceTest.cpp
 * @brief       Verifies the ported Robot HAT melody without audio hardware.
 *
 * @details
 * Checks source note order, measure boundaries, volume, tempo-derived duration,
 * PCM format, and callback validation through an in-memory music backend.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Host Test
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarToneSequence.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory backend and host verification scenario. */
namespace
{

/** @brief Records observable operations from the in-memory music backend. */
struct ToneState
{
    /** @brief Number of output-enable operations. */
    XWalkHal::uint32 enableCount{};
    /** @brief Number of volume operations. */
    XWalkHal::uint32 volumeCount{};
    /** @brief Last normalized volume value. */
    XWalkHal::float64 normalizedVolume{};
    /** @brief Number of generated tone payloads. */
    XWalkHal::uint32 toneCount{};
    /** @brief Total generated PCM bytes across the melody. */
    XWalkHal::size totalPcmBytes{};
    /** @brief Last generated sample rate. */
    XWalkHal::uint32 sampleRateHz{};
    /** @brief Last generated channel count. */
    XWalkHal::uint8 channelCount{};
    /** @brief Ordered measure reports. */
    XWalkHal::uint32vector measures;
};

/** @brief Records audio-output enablement. */
void enableOutput(XWalkHal::contextpointer context)
{
    ++static_cast<ToneState*>(context)->enableCount;
}

/** @brief Ignores an unused sound-effect request. */
void playSound(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::optionalfloat64 normalizedVolume)
{
    static_cast<void>(context);
    static_cast<void>(filename);
    static_cast<void>(normalizedVolume);
}

/** @brief Ignores an unused music-file request. */
void playMusic(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::int32 loops, XWalkHal::float64 startSeconds)
{
    static_cast<void>(context);
    static_cast<void>(filename);
    static_cast<void>(loops);
    static_cast<void>(startSeconds);
}

/** @brief Records the normalized volume selected by the sequence. */
void setMusicVolume(XWalkHal::contextpointer context,
    XWalkHal::float64 normalizedVolume)
{
    ToneState& state = *static_cast<ToneState*>(context);
    ++state.volumeCount;
    state.normalizedVolume = normalizedVolume;
}

/** @brief Ignores an unused music-control request. */
void controlMusic(XWalkHal::contextpointer context)
{
    static_cast<void>(context);
}

/** @brief Returns a valid unused sound duration. */
XWalkHal::float64 getSoundLength(
    XWalkHal::contextpointer context, XWalkHal::stringview filename)
{
    static_cast<void>(context);
    static_cast<void>(filename);
    return 0.0;
}

/** @brief Records generated PCM size and format without playing it. */
void playTone(XWalkHal::contextpointer context,
    const XWalkHal::bytevector& pcmData, XWalkHal::uint32 sampleRateHz,
    XWalkHal::uint8 channelCount)
{
    ToneState& state = *static_cast<ToneState*>(context);
    ++state.toneCount;
    state.totalPcmBytes += pcmData.size();
    state.sampleRateHz = sampleRateHz;
    state.channelCount = channelCount;
}

/** @brief Records one source measure boundary. */
void reportMeasure(XWalkHal::contextpointer context,
    XWalkHal::uint8 measureNumber)
{
    static_cast<ToneState*>(context)->measures.push_back(measureNumber);
}

/** @brief Returns a complete in-memory callback table. */
XWalkHal::XWalkMusicCallbacks callbacks()
{
    return {&enableOutput, &playSound, &playSound, &playMusic,
        &setMusicVolume, &controlMusic, &controlMusic, &controlMusic,
        &getSoundLength, &playTone};
}

/** @brief Executes and verifies the complete host-safe port. */
void runTest()
{
    ToneState state;
    XWalkHal::XWalkMusic music(&state, callbacks());
    xwalk::hal::test::XWalkToneSequence sequence(
        music, &state, &reportMeasure);
    sequence.run();

    XWalkHal::size expectedPcmBytes{};
    XWalkHal::float64 totalBeatValue{};
    for (const xwalk::hal::test::XWalkToneEvent& event :
        xwalk::hal::test::XWalkToneSequence::melody())
    {
        totalBeatValue += event.beatValue;
        expectedPcmBytes += music.getToneData(music.noteFrequencyHz(event.noteName),
            music.beatDurationSeconds(event.beatValue)).size();
    }

    assert(state.enableCount == 1U);
    assert(state.volumeCount == 1U);
    assert(state.normalizedVolume == 0.8);
    assert(state.toneCount == 72U);
    assert(totalBeatValue == 12.0);
    assert(state.totalPcmBytes == expectedPcmBytes);
    assert(state.sampleRateHz == XHAL_RPI5CAR_MUSIC_SAMPLE_RATE_HZ);
    assert(state.channelCount == 1U);
    assert(state.measures.size() == 17U);
    for (XWalkHal::size index = 0U; index < state.measures.size(); ++index)
    {
        assert(state.measures[index] == index + 1U);
    }

    const xwalk::hal::test::tonesequenceeventarray& melody =
        xwalk::hal::test::XWalkToneSequence::melody();
    assert(melody.front().measureNumber == 1U);
    assert(melody.front().noteName == "G4");
    assert(melody.front().beatValue == XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE);
    assert(melody[1U].noteName == "A#4");
    assert(melody[38U].measureNumber == 10U);
    assert(melody[38U].noteName == "F5");
    assert(melody.back().measureNumber == 17U);
    assert(melody.back().noteName == "G4");
    assert(melody.back().beatValue ==
        XHAL_RPI5CAR_MUSIC_QUARTER_NOTE + XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE);

    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::test::XWalkToneSequence invalidSequence(
            music, nullptr, nullptr);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe tone-sequence verification.
 *
 * @return Zero after every assertion passes.
 */
int xWalkToneSequenceHostTest()
{
    runTest();
    return 0;
}
