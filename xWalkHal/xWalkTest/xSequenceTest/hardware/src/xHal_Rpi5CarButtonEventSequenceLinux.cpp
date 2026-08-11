/******************************************************************************
 * @file        xHal_Rpi5CarButtonEventSequenceLinux.cpp
 * @brief       Implements Linux callbacks for the D0 button-event sequence.
 *
 * @details
 * Provides wall-clock timestamps, bounded sleeping, and trace output for
 * physical Robot HAT execution.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarButtonEventSequenceLinux.h"

#include "xHal_Rpi5CarButtonEventSequence.h"
#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarTrace.h"

#include <chrono>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test {

void XWalkButtonEventSequenceLinux::run(XWalkGpio &gpio,
                                        uint32 durationSeconds) {
  XWalkButtonEventSequence buttonEventSequence(
      gpio, this, &XWalkButtonEventSequenceLinux::wait,
      &XWalkButtonEventSequenceLinux::time,
      &XWalkButtonEventSequenceLinux::event);
  announce(durationSeconds);
  buttonEventSequence.run(durationSeconds);
}

void XWalkButtonEventSequenceLinux::announce(uint32 durationSeconds) const {
  XWALK_HAL_TRACE_UID1(RPI .386,
                       "Monitoring D0 (GPIO17) for %u seconds; press and "
                       "release the connected button",
                       durationSeconds);
}

void XWalkButtonEventSequenceLinux::wait(contextpointer context,
                                         uint32 durationMilliseconds) {
  static_cast<void>(context);
  common::sleepMilliseconds(durationMilliseconds);
}

float64 XWalkButtonEventSequenceLinux::time(contextpointer context) {
  static_cast<void>(context);
  const auto elapsed = std::chrono::system_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::duration<float64>>(elapsed)
      .count();
}

void XWalkButtonEventSequenceLinux::event(contextpointer context,
                                          boolean pressed,
                                          float64 timestampSeconds) {
  static_cast<void>(context);
  XWALK_HAL_TRACE_UID2(RPI .387, "Button event %s at %.6f seconds",
                       pressed ? "pressed" : "released", timestampSeconds);
}

} /* namespace xwalk::hal::test */
