/******************************************************************************
 * @file        xHal_Rpi5CarUtilsSimulation.cpp
 * @brief       Implements the side-effect-free xWalkUtils simulation.
 * @details     Verifies representative callback routing without Linux side
 *effects.
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarUtilsSimulation.h"
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarUtilsHostStub.h"
namespace xwalk::hal::sim {
int32 runUtilsSimulation() {
  XWalkUtilsHostStub hostStub;
  XWalkUtils utilities(&hostStub, hostStub.callbacks());
  utilities.info("utility simulation");
  utilities.setVolume(45);
  const XWalkCommandResult result = utilities.runCommand("status");
  const boolean succeeded =
      (hostStub.message() == "utility simulation") &&
      (hostStub.volumePercent() == 45U) && (hostStub.command() == "status") &&
      (result.status == 0) && utilities.commandExists("xwalk-tool") &&
      (utilities.ipAddress() == "192.0.2.10") &&
      (utilities.username() == "xwalk") &&
      (XWalkUtils::mapping(5.0, 0.0, 10.0, 0.0, 100.0) == 50.0);
  XWALK_HAL_TRACE_UID0(RPI .136, "xWalkUtils host simulation completed");
  return succeeded ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
