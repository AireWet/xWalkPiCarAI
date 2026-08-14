/******************************************************************************
 * @file        xHal_Rpi5CarAudioSimulationStub.cpp
 * @brief       Composes the device-free Audio simulation backend.
 *
 * @details
 * Injects the in-memory Audio operation table into the shared ALSA owner and
 * runs the bounded standalone handler without opening physical hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
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

#include "xHal_Rpi5CarAudioSimulation.h"

#include "xHal_Rpi5CarAudioHandler.h"
#include "xHal_Rpi5CarAudioHostStub.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim {

int32 runAudioSimulation() {
  XWALK_HAL_TRACE_UID0(RPI .094,
                       "Creating device-free Audio simulation backend");
  XWalkAudioHostStub hostStub;
  XWalkAudioAlsa audio(&hostStub, hostStub.operations(), "host-pcm",
                       "host-mixer", "PCM");
  XWalkAudioHandler handler;
  return handler.run(audio);
}

} /* namespace xwalk::hal::sim */
