/******************************************************************************
 * @file        xHal_Rpi5CarMusicTest.cpp
 * @brief       Verifies music behavior using named in-memory audio support.
 * @project     xWalk Firmware
 * @module      xWalkMusic Host Test
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarMusic.h"
#include "xHal_Rpi5CarMusicSimulationArguments.h"
#include "xHal_Rpi5CarMusicSimulationConfig.h"
#include "xHal_Rpi5CarMusicTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include <limits>
namespace {
using namespace xwalk::hal::test::music;
/** @brief Verifies default state, time signatures, tempo, and beat conversion.
 */
void testTiming() {
  TestBackend backend;
  const XWalkHal::XWalkMusicCallbacks callbacks = musicCallbacks();
  XWalkHal::XWalkMusic music(&backend, callbacks);
  assert(backend.enableCount == 1U);
  assert(music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_TOP_INDEX] ==
         4U);
  assert(
      music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_BOTTOM_INDEX] ==
      4U);
  assert(music.tempo()[XHAL_RPI5CAR_MUSIC_TEMPO_BPM_INDEX] == 120.0);
  assert(music.tempo()[XHAL_RPI5CAR_MUSIC_TEMPO_NOTE_VALUE_INDEX] ==
         XHAL_RPI5CAR_MUSIC_QUARTER_NOTE);
  assert(music.beatDurationSeconds(XHAL_RPI5CAR_MUSIC_QUARTER_NOTE) == 0.5);
  music.setTimeSignature(3U, 8U);
  assert(music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_TOP_INDEX] ==
         3U);
  assert(
      music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_BOTTOM_INDEX] ==
      8U);
  music.setTimeSignature(6U);
  assert(music.timeSignature()[XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_TOP_INDEX] ==
         6U);
  music.setTempo(60.0, XHAL_RPI5CAR_MUSIC_HALF_NOTE);
  assert(music.beatDurationSeconds(XHAL_RPI5CAR_MUSIC_WHOLE_NOTE) == 2.0);
}
/** @brief Verifies numeric and named note conversion with key displacement. */
void testNotes() {
  TestBackend backend;
  const XWalkHal::XWalkMusicCallbacks callbacks = musicCallbacks();
  XWalkHal::XWalkMusic music(&backend, callbacks);
  assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz(69) - 440.0) < 0.000001);
  assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz("A4") - 440.0) < 0.000001);
  music.setKeySignature(XHAL_RPI5CAR_MUSIC_KEY_G_MAJOR);
  assert(music.keySignature() == XHAL_RPI5CAR_MUSIC_KEY_G_MAJOR);
  assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz("A4") - 466.1637615) <
         0.0001);
  assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz("A4", true) - 440.0) <
         0.000001);
  music.setKeySignature("bbb");
  assert(music.keySignature() == -3);
  music.setKeySignature(0);
  assert(XHAL_ABSOLUTE_VALUE(music.noteFrequencyHz("C8") - 4'186.009045) <
         0.001);
}
/** @brief Verifies sound, streamed music, controls, and duration routing. */
void testPlayback() {
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
  assert(backend.soundVolume.has_value() == false);
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
void testToneGeneration() {
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
void testValidation() {
  TestBackend backend;
  XWalkHal::XWalkMusicCallbacks callbacks = musicCallbacks();
  callbacks.playTone = nullptr;
  xwalk::hal::test::expectFailure(
      [&]() { XWalkHal::XWalkMusic music(&backend, callbacks); });
  callbacks = musicCallbacks();
  XWalkHal::XWalkMusic music(&backend, callbacks);
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.noteFrequencyHz("H4")); });
  xwalk::hal::test::expectFailure([&]() { music.musicSetVolume(101.0); });
  xwalk::hal::test::expectFailure([&]() { music.setTimeSignature(0U, 4U); });
  xwalk::hal::test::expectFailure([&]() { music.setTimeSignature(4U, 0U); });
  xwalk::hal::test::expectFailure([&]() { music.setKeySignature(8); });
  xwalk::hal::test::expectFailure([&]() { music.setKeySignature(-8); });
  xwalk::hal::test::expectFailure([&]() { music.setKeySignature("########"); });
  xwalk::hal::test::expectFailure([&]() { music.setKeySignature("x"); });
  xwalk::hal::test::expectFailure([&]() { music.setKeySignature("#b"); });
  xwalk::hal::test::expectFailure([&]() {
    music.setTempo(std::numeric_limits<XWalkHal::float64>::quiet_NaN(), 1.0);
  });
  xwalk::hal::test::expectFailure([&]() {
    music.setTempo(60.0, std::numeric_limits<XWalkHal::float64>::infinity());
  });
  xwalk::hal::test::expectFailure([&]() { music.setTempo(0.0, 1.0); });
  xwalk::hal::test::expectFailure([&]() { music.setTempo(60.0, 0.0); });
  xwalk::hal::test::expectFailure([&]() {
    static_cast<void>(music.beatDurationSeconds(
        std::numeric_limits<XWalkHal::float64>::quiet_NaN()));
  });
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.beatDurationSeconds(-0.25)); });
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.noteFrequencyHz(-1)); });
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.noteFrequencyHz(109)); });
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.noteFrequencyHz("")); });
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.noteFrequencyHz("C##4")); });
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.noteFrequencyHz("C!4")); });
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.noteFrequencyHz("C9")); });
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.noteFrequencyHz("C0")); });
  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(music.noteFrequencyHz("D8")); });
  music.setKeySignature(-7);
  static_cast<void>(music.noteFrequencyHz(0));
  music.setKeySignature(7);
  static_cast<void>(music.noteFrequencyHz(108));
  music.setKeySignature("");
  static_cast<void>(music.noteFrequencyHz("D4"));
  static_cast<void>(music.noteFrequencyHz("E4"));
  static_cast<void>(music.noteFrequencyHz("F4"));
  static_cast<void>(music.noteFrequencyHz("G4"));
  static_cast<void>(music.noteFrequencyHz("B4"));
}
/** @brief Verifies persistent Music trace-selector behavior. */
void testTraceSelection() {
  char executable[] = "xWalkMusicTest";
  char option[] = "--trace";
  char enableSelector[] = "RPI.303.enable";
  char disableSelector[] = "RPI.303.disable";
  char malformedSelector[] = "RPI.invalid.enable";
  XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
  xwalk::hal::sim::XWalkMusicSimulationArguments enable(3, enableArguments);
  assert(enable.valid());
  assert(enable.applyTraceUpdate());
  XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
  xwalk::hal::sim::XWalkMusicSimulationArguments disable(3, disableArguments);
  assert(disable.valid());
  assert(disable.applyTraceUpdate());
  XWalkHal::charpointer malformedArguments[]{executable, option,
                                             malformedSelector};
  const xwalk::hal::sim::XWalkMusicSimulationArguments malformed(
      3, malformedArguments);
  assert(malformed.valid() == false);
}
} /* namespace */
/** @brief Runs all host-side music tests. */
XWalkHal::int32 main() {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_MUSIC_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_MUSIC_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .306, "xWalkMusic host tests started");
  testTiming();
  testNotes();
  testPlayback();
  testToneGeneration();
  testValidation();
  testTraceSelection();
  XWALK_HAL_TRACE_UID0(RPI .307, "xWalkMusic host tests completed");
  return 0;
}
