/******************************************************************************
 * @file        xAgent_Rpi5CarStorytellingRobotLifecycle.cpp
 * @brief       Implements storytelling construction, validation, and timing.
 * @project     xWalk Firmware
 * @module      xWalkStorytellingRobot
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarStorytellingRobot.h"

#include "xHal_Rpi5CarTrace.h"
#include <cmath>

namespace xwalk::agent {

XWalkStorytellingRobot::XWalkStorytellingRobot(
    XWalkPicarx &picarx, hal::XWalkTextToSpeech &textToSpeech,
    agent::contextpointer context,
    const XWalkStorytellingRobotCallbacks &backendCallbacks,
    const XWalkStorytellingRobotConfiguration &robotConfiguration)
    : picarxObject(&picarx), textToSpeechObject(&textToSpeech),
      callbackContext(context), callbacks(backendCallbacks),
      configuration(robotConfiguration) {
  validate(callbacks, configuration);
}

void XWalkStorytellingRobot::validate(
    const XWalkStorytellingRobotCallbacks &backendCallbacks,
    const XWalkStorytellingRobotConfiguration &robotConfiguration) {
  const agent::boolean backendCallbacksDelayShouldContinueInvalid =
      static_cast<agent::boolean>(
          (backendCallbacks.delay == nullptr) ||
          (backendCallbacks.shouldContinue == nullptr) ||
          robotConfiguration.greeting.empty() ||
          robotConfiguration.firstJoke.empty() ||
          robotConfiguration.secondJoke.empty() ||
          robotConfiguration.farewell.empty() ||
          !std::isfinite(robotConfiguration.speedPercent));
  if (backendCallbacksDelayShouldContinueInvalid) {
    XWALK_RPIAGENT_ERROR(XWALK_INVAL,
                         "Storytelling-robot configuration is invalid");
  }
  if ((robotConfiguration.speedPercent < 0.0) ||
      (robotConfiguration.speedPercent > 100.0) ||
      (robotConfiguration.outwardLegDurationMs == 0U) ||
      (robotConfiguration.homeLegDurationMs == 0U) ||
      (robotConfiguration.outwardLegDurationMs > 60'000U) ||
      (robotConfiguration.homeLegDurationMs > 60'000U)) {
    XWALK_RPIAGENT_ERROR(
        XWALK_RANGE, "Storytelling-robot configuration is outside its range");
  }
}

agent::boolean XWalkStorytellingRobot::wait(agent::uint32 durationMs) const {
  constexpr agent::uint32 cancellationIntervalMs{20U};
  agent::uint32 remainingMs = durationMs;
  while (remainingMs > 0U) {
    const agent::boolean operationRequested =
        callbacks.shouldContinue(callbackContext);
    if (operationRequested == false) {
      return false;
    }
    const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs)
                                      ? remainingMs
                                      : cancellationIntervalMs;
    callbacks.delay(callbackContext, sliceMs);
    remainingMs -= sliceMs;
  }
  return callbacks.shouldContinue(callbackContext);
}

} /* namespace xwalk::agent */
