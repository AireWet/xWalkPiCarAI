/******************************************************************************
 * @file        xHal_Rpi5CarMusicTheory.cpp
 * @brief       Implements music timing, key, and note-frequency behavior.
 *
 * @details
 * Validates time and tempo state, parses Python-compatible sharp note names,
 * applies key displacement, and calculates equal-temperament frequencies.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic
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

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Sets both values of the time signature to one value.
 *
 * @param[in] value
 * Non-zero upper and lower time-signature value.
 *
 * @post
 * `timeSignature()` returns `{value, value}`.
 *
 * @throws std::out_of_range
 * If `value` is zero.
 */
void XWalkMusic::setTimeSignature(uint32 value) {
  setTimeSignature(value, value);
}

/**
 * @brief Sets the upper and lower time-signature values.
 *
 * @param[in] top
 * Non-zero number of beats in one measure.
 *
 * @param[in] bottom
 * Non-zero note-value denominator receiving one beat.
 *
 * @post
 * `timeSignature()` returns the requested pair.
 *
 * @throws std::out_of_range
 * If either value is zero.
 */
void XWalkMusic::setTimeSignature(uint32 top, uint32 bottom) {
  if ((top == 0U) || (bottom == 0U)) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "Music time-signature values must be non-zero");
  }
  timeSignatureValue = {top, bottom};
  XWALK_HAL_TRACE_UID2(RPI .289, "Music time signature set to %u/%u", top,
                       bottom);
}

/**
 * @brief Returns the active time signature.
 *
 * @return
 * Upper beat count followed by lower note-value denominator.
 */
musictimesignature XWalkMusic::timeSignature() const noexcept {
  return timeSignatureValue;
}

/**
 * @brief Sets a signed key-signature displacement.
 *
 * @param[in] keySignature
 * Semitone displacement in the inclusive range minus seven to seven.
 *
 * @throws std::out_of_range
 * If the displacement is outside the supported range.
 */
void XWalkMusic::setKeySignature(int32 keySignature) {
  validateKeySignature(keySignature);
  keySignatureValue = keySignature;
  XWALK_HAL_TRACE_UID1(RPI .290, "Music key displacement set to %d",
                       keySignature);
}

/**
 * @brief Sets a sharp or flat key signature from text.
 *
 * @param[in] keySignature
 * Empty text for no displacement, or one through seven `#` or `b` characters.
 *
 * @throws std::invalid_argument
 * If the text mixes markers or contains another character.
 *
 * @throws std::out_of_range
 * If more than seven markers are supplied.
 */
void XWalkMusic::setKeySignature(stringview keySignature) {
  const hal::boolean keySignatureEmpty =
      static_cast<hal::boolean>(keySignature.empty());
  if (keySignatureEmpty) {
    setKeySignature(0);
    return;
  }
  const hal::boolean keySignatureTooLarge = static_cast<hal::boolean>(
      keySignature.size() >
      static_cast<size>(XHAL_RPI5CAR_MUSIC_MAXIMUM_KEY_SIGNATURE));
  if (keySignatureTooLarge) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "Music key signature supports at most seven markers");
  }

  const char marker = keySignature[0U];
  if ((marker != '#') && (marker != 'b')) {
    XWALK_HAL_ERROR(
        XWALK_INVAL,
        "Music key signature must contain only sharp or flat markers");
  }
  for (const char character : keySignature) {
    if (character != marker) {
      XWALK_HAL_ERROR(XWALK_INVAL,
                      "Music key signature must not mix marker types");
    }
  }

  const int32 markerCount = static_cast<int32>(keySignature.size());
  const int32 displacementDirection =
      (marker == '#') ? XHAL_RPI5CAR_MUSIC_KEY_SIGNATURE_SHARP
                      : XHAL_RPI5CAR_MUSIC_KEY_SIGNATURE_FLAT;
  const int32 displacement = markerCount * displacementDirection;
  setKeySignature(displacement);
}

/**
 * @brief Returns the active key displacement.
 *
 * @return
 * Signed semitone displacement in the inclusive range minus seven to seven.
 */
int32 XWalkMusic::keySignature() const noexcept { return keySignatureValue; }

/**
 * @brief Configures tempo and the note value receiving one beat.
 *
 * @param[in] beatsPerMinute
 * Finite tempo greater than zero, in beats per minute.
 *
 * @param[in] noteValue
 * Finite positive beat value as a fraction of one whole note.
 *
 * @post
 * `tempo()` returns the requested values and beat calculations use them.
 *
 * @throws std::invalid_argument
 * If either value is not finite or not greater than zero.
 */
void XWalkMusic::setTempo(float64 beatsPerMinute, float64 noteValue) {
  const hal::boolean beatsPerMinuteNoteInvalid = static_cast<hal::boolean>(
      !XHAL_IS_FINITE(beatsPerMinute) || !XHAL_IS_FINITE(noteValue));
  if (beatsPerMinuteNoteInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Music tempo values must be finite");
  }
  if ((beatsPerMinute <= 0.0) || (noteValue <= 0.0)) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Music tempo values must be greater than zero");
  }

  tempoValue = {beatsPerMinute, noteValue};
  beatUnitSecondsValue = XHAL_RPI5CAR_MUSIC_SECONDS_PER_MINUTE / beatsPerMinute;
  XWALK_HAL_TRACE_UID2(RPI .291,
                       "Music tempo set to %.2f BPM with note value %.4f",
                       beatsPerMinute, noteValue);
}

/**
 * @brief Returns the active tempo configuration.
 *
 * @return
 * Beats per minute followed by the whole-note beat value.
 */
musictempo XWalkMusic::tempo() const noexcept { return tempoValue; }

/**
 * @brief Converts a note-value duration to seconds at the active tempo.
 *
 * @param[in] beatValue
 * Finite non-negative duration expressed as a fraction of one whole note.
 *
 * @return
 * Corresponding non-negative duration in seconds.
 *
 * @throws std::invalid_argument
 * If the beat value is not finite.
 *
 * @throws std::out_of_range
 * If the beat value is negative.
 */
float64 XWalkMusic::beatDurationSeconds(float64 beatValue) const {
  const hal::boolean beatNotFinite =
      static_cast<hal::boolean>(!XHAL_IS_FINITE(beatValue));
  if (beatNotFinite) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Music beat value must be finite");
  }
  if (beatValue < 0.0) {
    XWALK_HAL_ERROR(XWALK_RANGE, "Music beat value must not be negative");
  }

  const float64 configuredNoteValue =
      tempoValue[XHAL_RPI5CAR_MUSIC_TEMPO_NOTE_VALUE_INDEX];
  const float64 configuredBeatCount = beatValue / configuredNoteValue;
  return configuredBeatCount * beatUnitSecondsValue;
}

/**
 * @brief Converts a MIDI-compatible note number to frequency.
 *
 * @param[in] noteIndex
 * MIDI note number in the inclusive range 0 to 108.
 *
 * @param[in] natural
 * `true` to ignore the active key signature; otherwise apply and clamp it.
 *
 * @return
 * Equal-temperament note frequency in Hertz.
 *
 * @throws std::out_of_range
 * If the supplied note number is outside the supported range.
 */
float64 XWalkMusic::noteFrequencyHz(int32 noteIndex, boolean natural) const {
  if ((noteIndex < XHAL_RPI5CAR_MUSIC_MINIMUM_NOTE_INDEX) ||
      (noteIndex > XHAL_RPI5CAR_MUSIC_MAXIMUM_NOTE_INDEX)) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "Music note index is outside the supported MIDI range");
  }

  int32 adjustedNoteIndex = noteIndex;
  if (!natural) {
    adjustedNoteIndex += keySignatureValue;
    if (adjustedNoteIndex < XHAL_RPI5CAR_MUSIC_MINIMUM_NOTE_INDEX) {
      adjustedNoteIndex = XHAL_RPI5CAR_MUSIC_MINIMUM_NOTE_INDEX;
    } else if (adjustedNoteIndex > XHAL_RPI5CAR_MUSIC_MAXIMUM_NOTE_INDEX) {
      adjustedNoteIndex = XHAL_RPI5CAR_MUSIC_MAXIMUM_NOTE_INDEX;
    }
  }

  const int32 noteDelta =
      adjustedNoteIndex - XHAL_RPI5CAR_MUSIC_NOTE_BASE_INDEX;
  const float64 noteDeltaValue = static_cast<float64>(noteDelta);
  const float64 octaveDisplacement =
      noteDeltaValue / XHAL_RPI5CAR_MUSIC_SEMITONES_PER_OCTAVE;
  const float64 frequencyRatio = XHAL_POWER(2.0, octaveDisplacement);
  return XHAL_RPI5CAR_MUSIC_NOTE_BASE_FREQUENCY_HZ * frequencyRatio;
}

/**
 * @brief Converts a Python-compatible note name to frequency.
 *
 * @param[in] noteName
 * Note from `A0` through `C8`, optionally containing one sharp marker.
 *
 * @param[in] natural
 * `true` to ignore the active key signature; otherwise apply and clamp it.
 *
 * @return
 * Equal-temperament note frequency in Hertz.
 *
 * @throws std::invalid_argument
 * If the note name is not represented by the Python-compatible note list.
 */
float64 XWalkMusic::noteFrequencyHz(stringview noteName,
                                    boolean natural) const {
  return noteFrequencyHz(parseNoteIndex(noteName), natural);
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Parses one Python-compatible sharp note name.
 *
 * @param[in] noteName
 * Note from `A0` through `C8`, optionally containing one sharp marker.
 *
 * @return
 * MIDI-compatible note number in the inclusive range 21 to 108.
 *
 * @throws std::invalid_argument
 * If the spelling or octave is not represented by the Python note list.
 */
int32 XWalkMusic::parseNoteIndex(stringview noteName) {
  const hal::boolean noteNameLengthInvalid = static_cast<hal::boolean>(
      (noteName.size() != 2U) && (noteName.size() != 3U));
  if (noteNameLengthInvalid) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Music note name has an unsupported length");
  }

  int32 pitchClass{};
  switch (noteName[0U]) {
  case 'C':
    pitchClass = 0;
    break;
  case 'D':
    pitchClass = 2;
    break;
  case 'E':
    pitchClass = 4;
    break;
  case 'F':
    pitchClass = 5;
    break;
  case 'G':
    pitchClass = 7;
    break;
  case 'A':
    pitchClass = 9;
    break;
  case 'B':
    pitchClass = 11;
    break;
  default:
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Music note name requires an uppercase pitch letter");
  }

  size octaveCharacterIndex = 1U;
  const hal::boolean noteNameMatched =
      static_cast<hal::boolean>(noteName.size() == 3U);
  if (noteNameMatched) {
    if (noteName[1U] != '#') {
      XWALK_HAL_ERROR(XWALK_INVAL,
                      "Music note names support only sharp accidentals");
    }
    ++pitchClass;
    octaveCharacterIndex = 2U;
  }

  const char octaveCharacter = noteName[octaveCharacterIndex];
  if ((octaveCharacter < '0') || (octaveCharacter > '8')) {
    XWALK_HAL_ERROR(
        XWALK_INVAL,
        "Music note octave must be in the range zero through eight");
  }

  const int32 octave = static_cast<int32>(octaveCharacter - '0');
  const int32 midiOctave = octave + 1;
  const int32 octaveBase = midiOctave * 12;
  const int32 noteIndex = octaveBase + pitchClass;
  if ((noteIndex < XHAL_RPI5CAR_MUSIC_FIRST_NAMED_NOTE_INDEX) ||
      (noteIndex > XHAL_RPI5CAR_MUSIC_MAXIMUM_NOTE_INDEX)) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Music note is outside the named range A0 through C8");
  }
  return noteIndex;
}

/**
 * @brief Validates a key signature expressed as a semitone displacement.
 *
 * @param[in] keySignature
 * Signed semitone displacement in the inclusive range minus seven to seven.
 *
 * @throws std::out_of_range
 * If the displacement is outside the supported range.
 */
void XWalkMusic::validateKeySignature(int32 keySignature) {
  if ((keySignature < XHAL_RPI5CAR_MUSIC_MINIMUM_KEY_SIGNATURE) ||
      (keySignature > XHAL_RPI5CAR_MUSIC_MAXIMUM_KEY_SIGNATURE)) {
    XWALK_HAL_ERROR(
        XWALK_RANGE,
        "Music key signature must be between minus seven and seven");
  }
}

} /* namespace xwalk::hal */
