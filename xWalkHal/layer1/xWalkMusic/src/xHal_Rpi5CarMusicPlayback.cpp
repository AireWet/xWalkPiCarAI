/******************************************************************************
 * @file        xHal_Rpi5CarMusicPlayback.cpp
 * @brief       Implements injected sound and streamed-music playback control.
 *
 * @details
 * Validates filenames, volume, loop, and offset values before forwarding sound
 * effect and streamed-music operations to the caller-owned callback backend.
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
void XWalkMusic::soundPlay(stringview filename, optionalfloat64 volumePercent) {
  validateFilename(filename);
  optionalfloat64 normalizedVolumeValue{};
  const hal::boolean volumePercentProvided =
      static_cast<hal::boolean>(volumePercent.has_value());
  if (volumePercentProvided) {
    normalizedVolumeValue = normalizedVolume(*volumePercent);
  }
  callbacks.playSound(backendContext, filename, normalizedVolumeValue);
  XWALK_HAL_TRACE_UID2(RPI .292,
                       "Synchronous sound playback completed for %.*s",
                       static_cast<int32>(filename.size()), filename.data());
}

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
void XWalkMusic::soundPlayBackground(stringview filename,
                                     optionalfloat64 volumePercent) {
  validateFilename(filename);
  optionalfloat64 normalizedVolumeValue{};
  const hal::boolean volumePercentProvided =
      static_cast<hal::boolean>(volumePercent.has_value());
  if (volumePercentProvided) {
    normalizedVolumeValue = normalizedVolume(*volumePercent);
  }
  callbacks.playSoundBackground(backendContext, filename,
                                normalizedVolumeValue);
  XWALK_HAL_TRACE_UID2(RPI .293, "Background sound playback started for %.*s",
                       static_cast<int32>(filename.size()), filename.data());
}

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
void XWalkMusic::musicPlay(stringview filename, int32 loops,
                           float64 startSeconds,
                           optionalfloat64 volumePercent) {
  validateFilename(filename);
  const hal::boolean startSecondsNotFinite =
      static_cast<hal::boolean>(!XHAL_IS_FINITE(startSeconds));
  if (startSecondsNotFinite) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Music playback offset must be finite");
  }
  if ((loops < 0) || (startSeconds < 0.0)) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "Music playback loops and offset must not be negative");
  }
  const hal::boolean volumePercentProvided =
      static_cast<hal::boolean>(volumePercent.has_value());
  if (volumePercentProvided) {
    musicSetVolume(*volumePercent);
  }
  callbacks.playMusic(backendContext, filename, loops, startSeconds);
  XWALK_HAL_TRACE_UID4(RPI .294,
                       "Streamed music playback started for %.*s with %d "
                       "loop(s) at %.3f seconds",
                       static_cast<int32>(filename.size()), filename.data(),
                       loops, startSeconds);
}

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
void XWalkMusic::musicSetVolume(float64 volumePercent) {
  callbacks.setMusicVolume(backendContext, normalizedVolume(volumePercent));
  XWALK_HAL_TRACE_UID1(RPI .295, "Streamed music volume set to %.2f percent",
                       volumePercent);
}

/**
 * @brief Stops the current streamed-music operation.
 *
 * @post
 * The backend stop callback has completed.
 */
void XWalkMusic::musicStop() {
  callbacks.stopMusic(backendContext);
  XWALK_HAL_TRACE_UID0(RPI .296, "Streamed music stopped");
}

/**
 * @brief Pauses the current streamed-music operation.
 *
 * @post
 * The backend pause callback has completed.
 */
void XWalkMusic::musicPause() {
  callbacks.pauseMusic(backendContext);
  XWALK_HAL_TRACE_UID0(RPI .297, "Streamed music paused");
}

/**
 * @brief Resumes the current paused streamed-music operation.
 *
 * @post
 * The backend resume callback has completed.
 */
void XWalkMusic::musicResume() {
  callbacks.resumeMusic(backendContext);
  XWALK_HAL_TRACE_UID0(RPI .298, "Streamed music resumed");
}

/**
 * @brief Provides the Python-compatible alias for `musicResume()`.
 *
 * @post
 * The backend resume callback has completed.
 */
void XWalkMusic::musicUnpause() { musicResume(); }

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
float64 XWalkMusic::soundLength(stringview filename) {
  validateFilename(filename);
  const float64 durationSeconds =
      callbacks.getSoundLength(backendContext, filename);
  const hal::boolean durationSecondsInvalid = static_cast<hal::boolean>(
      !XHAL_IS_FINITE(durationSeconds) || (durationSeconds < 0.0));
  if (durationSecondsInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Music backend returned an invalid sound duration");
  }
  const float64 roundedDurationSeconds = roundedHundredths(durationSeconds);
  XWALK_HAL_TRACE_UID3(RPI .299, "Sound length for %.*s is %.2f seconds",
                       static_cast<int32>(filename.size()), filename.data(),
                       roundedDurationSeconds);
  return roundedDurationSeconds;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates a non-empty audio filename.
 *
 * @param[in] filename
 * File path view that must contain at least one character.
 *
 * @throws std::invalid_argument
 * If the path view is empty.
 */
void XWalkMusic::validateFilename(stringview filename) {
  const hal::boolean filenameEmpty =
      static_cast<hal::boolean>(filename.empty());
  if (filenameEmpty) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Music filename must not be empty");
  }
}

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
float64 XWalkMusic::normalizedVolume(float64 volumePercent) {
  const hal::boolean volumePercentNotFinite =
      static_cast<hal::boolean>(!XHAL_IS_FINITE(volumePercent));
  if (volumePercentNotFinite) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Music volume must be finite");
  }
  if ((volumePercent < XHAL_RPI5CAR_MUSIC_MINIMUM_VOLUME_PERCENT) ||
      (volumePercent > XHAL_RPI5CAR_MUSIC_MAXIMUM_VOLUME_PERCENT)) {
    XWALK_HAL_ERROR(
        XWALK_RANGE,
        "Music volume must be between zero and one hundred percent");
  }
  const float64 normalizedValue =
      volumePercent / XHAL_RPI5CAR_MUSIC_VOLUME_PERCENT_DIVISOR;
  return roundedHundredths(normalizedValue);
}

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
float64 XWalkMusic::roundedHundredths(float64 value) {
  const float64 scaledValue = value * XHAL_RPI5CAR_MUSIC_HUNDREDTH_SCALE;
  const float64 roundedValue = XHAL_ROUND_NEAREST(scaledValue);
  return roundedValue / XHAL_RPI5CAR_MUSIC_HUNDREDTH_SCALE;
}

} /* namespace xwalk::hal */
