/******************************************************************************
 * @file        xHal_Rpi5CarToneSequence.cpp
 * @brief       Implements the Robot HAT tone melody sequence.
 *
 * @details
 * Retains the active notes, measures, tempo, volume, and synchronous playback
 * order from robot-hat/tests/tone_test.py.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest
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

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

/**
 * @brief Binds one music controller and measure reporter.
 *
 * @param[in,out] music Caller-owned music controller.
 * @param[in,out] context Context forwarded to `reportMeasure`.
 * @param[in] reportMeasure Non-null measure callback.
 *
 * @throws std::invalid_argument If `reportMeasure` is null.
 */
XWalkToneSequence::XWalkToneSequence(XWalkMusic& music, contextpointer context,
    tonesequencemeasurecallback reportMeasure):
    musicObject(&music), measureContext(context), measureCallback(reportMeasure)
{
    if (measureCallback == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Tone sequence requires a measure callback");
    }
}

/**
 * @brief Returns the immutable 72-note source melody.
 *
 * @return Stable melody definition grouped into 17 measures.
 */
const tonesequenceeventarray& XWalkToneSequence::melody() noexcept
{
    static const tonesequenceeventarray events{{
        {1U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {2U, "A#4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {2U, "C5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {2U, "D5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {2U, "D#5", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {2U, "D5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {3U, "C5", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {3U, "A4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {3U, "F4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {3U, "G4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {3U, "A4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {4U, "A#4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {4U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {4U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {4U, "F#4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {4U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {5U, "A4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {5U, "F#4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {5U, "D4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {5U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {6U, "A#4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {6U, "C5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {6U, "D5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {6U, "D#5", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {6U, "D5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {7U, "C5", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {7U, "A4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {7U, "F4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {7U, "G4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {7U, "A4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {8U, "A#4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {8U, "A4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {8U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {8U, "F#4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {8U, "E4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {8U, "F#4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {9U, "G4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE + XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {9U, "G4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE + XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {10U, "F5", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE + XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {10U, "F5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE}, {10U, "E5", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {10U, "D5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {11U, "C5", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {11U, "A4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {11U, "F4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {11U, "G4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {11U, "A4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {12U, "A#4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {12U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {12U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {12U, "F#4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {12U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {13U, "A4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {13U, "F#4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {13U, "D4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE + XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {14U, "F5", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE + XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {14U, "F5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE}, {14U, "E5", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {14U, "D5", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {15U, "C5", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE}, {15U, "A4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {15U, "F4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {15U, "G4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {15U, "A4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {16U, "A#4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {16U, "A4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE}, {16U, "G4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {16U, "F#4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE + XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {16U, "E4", XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE},
        {16U, "F#4", XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {17U, "G4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE + XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE},
        {17U, "G4", XHAL_RPI5CAR_MUSIC_QUARTER_NOTE + XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE}
    }};
    return events;
}

/**
 * @brief Plays every source measure synchronously.
 *
 * @post Volume, tempo, reporting order, and all 72 tone submissions match the
 * enabled Python sequence.
 */
void XWalkToneSequence::run()
{
    musicObject->musicSetVolume(80.0);
    musicObject->setTempo(60.0, XHAL_RPI5CAR_MUSIC_QUARTER_NOTE);

    uint8 currentMeasure{};
    for (const XWalkToneEvent& event : melody())
    {
        if (event.measureNumber != currentMeasure)
        {
            currentMeasure = event.measureNumber;
            measureCallback(measureContext, currentMeasure);
        }
        musicObject->playToneFor(musicObject->noteFrequencyHz(event.noteName),
            musicObject->beatDurationSeconds(event.beatValue));
    }
}

} /* namespace xwalk::hal::test */
