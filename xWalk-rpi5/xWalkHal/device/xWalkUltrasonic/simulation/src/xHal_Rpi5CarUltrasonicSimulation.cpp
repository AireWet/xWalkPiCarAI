/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicSimulation.cpp
 * @brief       Implements the device-free xWalkUltrasonic simulation.
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarUltrasonicSimulation.h"
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarUltrasonic.h"
#include "xHal_Rpi5CarUltrasonicHostStub.h"
namespace xwalk::hal::sim {
int32 runUltrasonicSimulation() {
  XWalkUltrasonicHostStub backend;
  const XWalkGpioCallbacks callbackSet = XWalkUltrasonicHostStub::callbacks();
  XWalkGpio trigger(&backend, callbackSet, "D2");
  XWalkGpio echo(&backend, callbackSet, "D3");
  XWalkUltrasonic ultrasonic(trigger, echo);
  const float64 distance = ultrasonic.read(1U);
  const boolean valid = (distance > 10.0) && (distance < 50.0) &&
                        (backend.triggerCount() == 1U) &&
                        (backend.triggerWriteCount() == 3U);
  XWALK_HAL_TRACE_UID0(RPI .207, "xWalkUltrasonic host simulation completed");
  return valid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
