/******************************************************************************
 * @file        xHal_Rpi5CarMusicSimulation.cpp
 * @brief       Implements the silent device-free xWalkMusic simulation.
 * @project     xWalk Firmware
 * @module      xWalkMusic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarMusicSimulation.h"
#include "xHal_Rpi5CarMusicHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
int32 runMusicSimulation() {
  XWalkMusicHostStub backend;
  const XWalkMusicCallbacks callbackSet = XWalkMusicHostStub::callbacks();
  XWalkMusic music(&backend, callbackSet);
  music.setTimeSignature(3U, 4U);
  music.setTempo(90.0);
  music.setKeySignature(XHAL_RPI5CAR_MUSIC_KEY_G_MAJOR);
  music.soundPlay("effect.wav", 25.0);
  music.soundPlayBackground("background.wav");
  music.musicPlay("music.wav", 1, 0.0, 50.0);
  music.musicPause();
  music.musicResume();
  music.musicStop();
  const float64 soundSeconds = music.soundLength("effect.wav");
  music.playToneFor(440.0, 0.001);
  const boolean valid =
      (backend.enableCount() == 1U) && (backend.playbackCount() == 3U) &&
      (backend.controlCount() == 4U) && (backend.toneCount() == 1U) &&
      (backend.toneByteCount() > 0U) && (soundSeconds == 1.25);
  XWALK_HAL_TRACE_UID0(RPI .303, "xWalkMusic host simulation completed");
  return valid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
