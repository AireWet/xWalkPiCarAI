/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerSimulation.cpp
 * @brief       Implements the silent device-free xWalkSpeaker simulation.
 * @project     xWalk Firmware
 * @module      xWalkSpeaker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarSpeakerSimulation.h"
#include "xHal_Rpi5CarSpeakerHostStub.h"
#include "xHal_Rpi5CarSpeakerSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
int32 runSpeakerSimulation() {
  const filesystempath fixturePath(XWALK_SPEAKER_SIMULATION_AUDIO_PATH);
  {
    outputfilestream fixture(fixturePath, FILE_OPEN_WRITE_TRUNCATE);
    const boolean fixtureOpen = fixture.is_open();
    if (fixtureOpen == false) {
      return 1;
    }
  }
  XWalkSpeakerHostStub backend;
  boolean taskCompleted = false;
  {
    const XWalkSpeakerCallbacks callbackSet = XWalkSpeakerHostStub::callbacks();
    XWalkSpeaker speaker(&backend, callbackSet);
    static_cast<void>(speaker.play(fixturePath.string()));
    common::sleepMilliseconds(5U);
    taskCompleted = speaker.listTasks().empty();
  }
  static_cast<void>(removeFilesystemEntry(fixturePath));
  const boolean valid =
      taskCompleted && (backend.enableCount() == 1U) &&
      (backend.disableCount() == 1U) && (backend.decodeCount() == 1U) &&
      (backend.openCount() == 1U) && (backend.writeCount() == 2U) &&
      (backend.closeCount() == 1U);
  XWALK_HAL_TRACE_UID0(RPI .316, "xWalkSpeaker host simulation completed");
  return valid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
