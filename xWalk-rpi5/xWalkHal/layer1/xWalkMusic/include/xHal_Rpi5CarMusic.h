/******************************************************************************
 * @file        xHal_Rpi5CarMusic.h
 * @brief       Declares music theory, tone generation, and playback control.
 *
 * @details
 * Provides Python-compatible timing, key, note-frequency, sound-effect,
 * streamed-music, and generated-tone behavior through an injected backend.
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

#ifndef XHAL_RPI5CAR_MUSIC_H
#define XHAL_RPI5CAR_MUSIC_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMusicTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkMusic
     * @brief Calculates musical values and coordinates a platform audio backend.
     *
     * @details
     * Stores time, tempo, and key state; converts MIDI-compatible notes to equal-
     * temperament frequencies; generates signed 16-bit PCM tones; and forwards
     * file and PCM operations through a caller-owned callback backend.
     */
    class XWalkMusic
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Nullable non-owning context passed to every audio callback.
             *
             * @details
             * Null is permitted when supported by the backend. Any non-null object
             * is created by the application and must outlive this controller.
             */
            contextpointer backendContext{nullptr};

            /** @brief Complete callback table copied from the caller during construction. */
            XWalkMusicCallbacks callbacks{};

            /** @brief Positive upper and lower values of the active time signature. */
            musictimesignature timeSignatureValue{XHAL_RPI5CAR_MUSIC_DEFAULT_TIME_SIGNATURE_TOP,
                                                  XHAL_RPI5CAR_MUSIC_DEFAULT_TIME_SIGNATURE_BOTTOM};

            /** @brief Positive tempo in beats per minute and positive whole-note beat value. */
            musictempo tempoValue{XHAL_RPI5CAR_MUSIC_DEFAULT_TEMPO_BPM, XHAL_RPI5CAR_MUSIC_QUARTER_NOTE};

            /** @brief Duration of one configured tempo beat in seconds. */
            float64 beatUnitSecondsValue{XHAL_RPI5CAR_MUSIC_SECONDS_PER_MINUTE / XHAL_RPI5CAR_MUSIC_DEFAULT_TEMPO_BPM};

            /** @brief Signed semitone displacement in the inclusive range minus seven to seven. */
            int32 keySignatureValue{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Validates the complete injected callback table.
             *
             * @param[in] backendCallbacks
             * Callback table whose entries must all be non-null.
             *
             * @throws std::invalid_argument
             * If any required callback is null.
             */
            static void validateCallbacks(const XWalkMusicCallbacks& backendCallbacks);

            /**
             * @brief Validates a non-empty audio filename.
             *
             * @param[in] filename
             * File path view that must contain at least one character.
             *
             * @throws std::invalid_argument
             * If the path view is empty.
             */
            static void validateFilename(stringview filename);

            /**
             * @brief Converts percent volume to a rounded normalized value.
             *
             * @param[in] volumePercent
             * Finite volume in the inclusive range 0.0 to 100.0 percent.
             *
             * @return
             * Volume rounded to hundredths in the inclusive range 0.0 to 1.0.
             *
             * @throws std::invalid_argument
             * If the volume is not finite.
             *
             * @throws std::out_of_range
             * If the volume is outside the supported percent range.
             */
            static float64 normalizedVolume(float64 volumePercent);

            /**
             * @brief Rounds a finite value to two decimal places.
             *
             * @param[in] value
             * Finite floating-point value.
             *
             * @return
             * Nearest value representable at hundredth precision.
             *
             * @pre
             * `value` is finite.
             */
            static float64 roundedHundredths(float64 value);

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
            static int32 parseNoteIndex(stringview noteName);

            /**
             * @brief Validates a key signature expressed as a semitone displacement.
             *
             * @param[in] keySignature
             * Signed semitone displacement in the inclusive range minus seven to seven.
             *
             * @throws std::out_of_range
             * If the displacement is outside the supported range.
             */
            static void validateKeySignature(int32 keySignature);

            /**
             * @brief Validates generated-tone frequency and duration.
             *
             * @param[in] frequencyHz
             * Finite frequency greater than zero, in Hertz.
             *
             * @param[in] durationSeconds
             * Finite non-negative requested duration in seconds.
             *
             * @throws std::invalid_argument
             * If either argument is not finite or the frequency is not positive.
             *
             * @throws std::out_of_range
             * If the duration is negative.
             */
            static void validateTone(float64 frequencyHz, float64 durationSeconds);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a music controller and enables its audio output.
             *
             * @param[in,out] context
             * Non-owning backend context; nullability is backend-specific.
             *
             * @param[in] backendCallbacks
             * Complete callback table copied into the controller.
             *
             * @pre
             * Any non-null context object outlives this controller.
             *
             * @post
             * The backend enable callback has completed, and default state is 4/4,
             * 120 quarter-note beats per minute, with no key displacement.
             *
             * @throws std::invalid_argument
             * If any required callback is null.
             */
            XWalkMusic(contextpointer context, const XWalkMusicCallbacks& backendCallbacks);

            /**
             * @brief Destroys the controller without releasing its caller-owned backend.
             *
             * @note
             * The Python implementation does not disable the speaker at destruction,
             * so this port intentionally leaves platform output state unchanged.
             */
            ~XWalkMusic();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction to preserve callback-context identity. */
            XWalkMusic(XWalkMusic&&) = delete;
            /** @brief Disables copying of the non-owning backend binding. */
            XWalkMusic(const XWalkMusic&) = delete;
            /** @brief Disables move assignment of the callback-context binding. */
            XWalkMusic& operator=(XWalkMusic&&) = delete;
            /** @brief Disables copy assignment of the callback-context binding. */
            XWalkMusic& operator=(const XWalkMusic&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

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
            void setTimeSignature(uint32 value);

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
            void setTimeSignature(uint32 top, uint32 bottom);

            /**
             * @brief Returns the active time signature.
             *
             * @return
             * Upper beat count followed by lower note-value denominator.
             */
            musictimesignature timeSignature() const noexcept;

            /**
             * @brief Sets a signed key-signature displacement.
             *
             * @param[in] keySignature
             * Semitone displacement in the inclusive range minus seven to seven.
             *
             * @throws std::out_of_range
             * If the displacement is outside the supported range.
             */
            void setKeySignature(int32 keySignature);

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
            void setKeySignature(stringview keySignature);

            /**
             * @brief Returns the active key displacement.
             *
             * @return
             * Signed semitone displacement in the inclusive range minus seven to seven.
             */
            int32 keySignature() const noexcept;

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
            void setTempo(float64 beatsPerMinute, float64 noteValue = XHAL_RPI5CAR_MUSIC_QUARTER_NOTE);

            /**
             * @brief Returns the active tempo configuration.
             *
             * @return
             * Beats per minute followed by the whole-note beat value.
             */
            musictempo tempo() const noexcept;

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
            float64 beatDurationSeconds(float64 beatValue) const;

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
            float64 noteFrequencyHz(int32 noteIndex, boolean natural = false) const;

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
            float64 noteFrequencyHz(stringview noteName, boolean natural = false) const;

            /**
             * @brief Plays a sound effect and waits for backend completion.
             *
             * @param[in] filename
             * Non-empty sound-effect path view.
             *
             * @param[in] volumePercent
             * Optional finite volume in the inclusive range 0.0 to 100.0 percent.
             *
             * @throws std::invalid_argument
             * If the filename is empty or volume is not finite.
             *
             * @throws std::out_of_range
             * If volume is outside the supported percent range.
             */
            void soundPlay(stringview filename, optionalfloat64 volumePercent = {});

            /**
             * @brief Starts sound-effect playback without waiting for completion.
             *
             * @param[in] filename
             * Non-empty sound-effect path view.
             *
             * @param[in] volumePercent
             * Optional finite volume in the inclusive range 0.0 to 100.0 percent.
             *
             * @throws std::invalid_argument
             * If the filename is empty or volume is not finite.
             *
             * @throws std::out_of_range
             * If volume is outside the supported percent range.
             */
            void soundPlayBackground(stringview filename, optionalfloat64 volumePercent = {});

            /**
             * @brief Starts streamed music playback.
             *
             * @param[in] filename
             * Non-empty music-file path view.
             *
             * @param[in] loops
             * Non-negative Python-compatible loop argument forwarded unchanged.
             *
             * @param[in] startSeconds
             * Finite non-negative playback offset in seconds.
             *
             * @param[in] volumePercent
             * Optional finite volume in the inclusive range 0.0 to 100.0 percent.
             *
             * @throws std::invalid_argument
             * If the filename is empty, or the start offset or volume is not finite.
             *
             * @throws std::out_of_range
             * If loops or the start offset is negative, or volume is outside its range.
             */
            void musicPlay(stringview filename,
                           int32 loops = 1,
                           float64 startSeconds = 0.0,
                           optionalfloat64 volumePercent = {});

            /**
             * @brief Sets the streamed-music volume.
             *
             * @param[in] volumePercent
             * Finite volume in the inclusive range 0.0 to 100.0 percent.
             *
             * @throws std::invalid_argument
             * If the volume is not finite.
             *
             * @throws std::out_of_range
             * If the volume is outside the supported percent range.
             */
            void musicSetVolume(float64 volumePercent);

            /**
             * @brief Stops the current streamed-music operation.
             *
             * @post
             * The backend stop callback has completed.
             */
            void musicStop();

            /**
             * @brief Pauses the current streamed-music operation.
             *
             * @post
             * The backend pause callback has completed.
             */
            void musicPause();

            /**
             * @brief Resumes the current paused streamed-music operation.
             *
             * @post
             * The backend resume callback has completed.
             */
            void musicResume();

            /**
             * @brief Provides the Python-compatible alias for `musicResume()`.
             *
             * @post
             * The backend resume callback has completed.
             */
            void musicUnpause();

            /**
             * @brief Returns one sound-effect duration rounded to hundredths.
             *
             * @param[in] filename
             * Non-empty sound-effect path view.
             *
             * @return
             * Finite non-negative duration in seconds, rounded to two decimal places.
             *
             * @throws std::invalid_argument
             * If the filename is empty.
             *
             * @throws std::runtime_error
             * If the backend returns a non-finite or negative duration.
             */
            float64 soundLength(stringview filename);

            /**
             * @brief Generates Python-compatible signed 16-bit mono PCM tone data.
             *
             * @param[in] frequencyHz
             * Finite frequency greater than zero, in Hertz.
             *
             * @param[in] durationSeconds
             * Finite non-negative requested duration in seconds.
             *
             * @return
             * Little-endian PCM bytes at 44,100 Hertz.
             *
             * @note
             * Compatibility requires halving the requested duration and appending
             * `frameCount % sampleRate` silent samples, matching the Python source.
             *
             * @throws std::invalid_argument
             * If frequency or duration is not finite, or frequency is not positive.
             *
             * @throws std::out_of_range
             * If duration is negative or the generated byte count exceeds the supported range.
             */
            bytevector getToneData(float64 frequencyHz, float64 durationSeconds) const;

            /**
             * @brief Generates and writes one tone through the injected backend.
             *
             * @param[in] frequencyHz
             * Finite frequency greater than zero, in Hertz.
             *
             * @param[in] durationSeconds
             * Finite non-negative requested duration in seconds.
             *
             * @throws std::invalid_argument
             * If frequency or duration is not finite, or frequency is not positive.
             *
             * @throws std::out_of_range
             * If duration is negative or the generated byte count exceeds the supported range.
             */
            void playToneFor(float64 frequencyHz, float64 durationSeconds);
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_MUSIC_H */
