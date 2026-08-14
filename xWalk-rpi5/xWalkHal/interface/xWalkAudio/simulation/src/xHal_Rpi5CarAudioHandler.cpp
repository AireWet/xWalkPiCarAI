/******************************************************************************
 * @file        xHal_Rpi5CarAudioHandler.cpp
 * @brief       Implements the standalone Audio simulation handler.
 *
 * @details
 * Performs one bounded silent playback through the public shared ALSA owner so
 * both simulation compositions exercise the same Audio behavior.
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

#include "xHal_Rpi5CarAudioHandler.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim {

XWalkAudioHandler::XWalkAudioHandler() = default;
XWalkAudioHandler::~XWalkAudioHandler() = default;

int32 XWalkAudioHandler::run(XWalkAudioAlsa &audio) const {
  const XWalkAudioStreamConfiguration configuration{
      44'100U, 1U, XWalkAudioSampleFormat::Signed16LittleEndian, 256U,
      XHAL_RPI5CAR_AUDIO_DEFAULT_LATENCY_US};
  const size frameCount = 256U;
  const size bytesPerFrame = 2U;
  const bytevector silence(frameCount * bytesPerFrame, 0U);
  audiopcmhandle stream = audio.openStream(configuration);
  audio.writeFrames(stream, silence, frameCount);
  audio.setVolume(50U);
  audio.closeStream(stream);
  XWALK_HAL_TRACE_UID1(
      RPI .093,
      "xWalkAudio simulation completed silent playback of %zu frame(s)",
      frameCount);
  return 0;
}

} /* namespace xwalk::hal::sim */
