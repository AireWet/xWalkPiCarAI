/******************************************************************************
 * @file        xAgent_Rpi5CarAppControl.cpp
 * @brief       Implements one bounded mobile-app control iteration.
 * @project     xWalk Firmware
 * @module      xWalkAppControl
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarAppControl.h"

#include "xHal_Rpi5CarTrace.h"
#include <algorithm>

namespace xwalk::agent {

agent::boolean XWalkAppControl::applyVoice(const agent::string &command) {
  if (command == "forward") {
    picarxObject->forward(speedPercentValue);
  } else if (command == "backward") {
    picarxObject->backward(speedPercentValue);
  } else if ((command == "left") || (command == "right") ||
             (command == "white") || (command == "rice")) {
    const agent::boolean left = command == "left";
    picarxObject->setDirectionServoAngle(left ? -30.0 : 30.0);
    picarxObject->forward(60.0);
    const agent::boolean delayCompleted = wait(1'200U);
    if (delayCompleted == false) {
      picarxObject->stop();
      return false;
    } else {
      picarxObject->setDirectionServoAngle(0.0);
      picarxObject->forward(speedPercentValue);
    }
  } else if (command == "stop") {
    picarxObject->stop();
  }
  return true;
}

agent::boolean XWalkAppControl::applyLineTracking() {
  constexpr agent::uint8 stopState{0U};
  constexpr agent::uint8 forwardState{1U};
  constexpr agent::uint8 rightState{2U};
  constexpr agent::uint8 leftState{3U};
  hal::linetrackervalues readings = picarxObject->grayscaleData();
  hal::linetrackerstatus status = picarxObject->lineStatus(readings);
  agent::uint8 state = stopState;
  if (status[1U]) {
    state = forwardState;
  } else if (status[0U]) {
    state = rightState;
  } else if (status[2U]) {
    state = leftState;
  }

  if (state != stopState) {
    lastLineState = state;
  }
  if (state == forwardState) {
    picarxObject->setDirectionServoAngle(0.0);
    picarxObject->forward(configurationValue.lineTrackingSpeedPercent);
    return true;
  }
  if (state == leftState) {
    picarxObject->setDirectionServoAngle(
        configurationValue.lineTrackingAngleDegrees);
    picarxObject->forward(configurationValue.lineTrackingSpeedPercent);
    return true;
  }
  if (state == rightState) {
    picarxObject->setDirectionServoAngle(
        -configurationValue.lineTrackingAngleDegrees);
    picarxObject->forward(configurationValue.lineTrackingSpeedPercent);
    return true;
  }

  if (lastLineState == leftState) {
    picarxObject->setDirectionServoAngle(-30.0);
    picarxObject->backward(10.0);
  } else if (lastLineState == rightState) {
    picarxObject->setDirectionServoAngle(30.0);
    picarxObject->backward(10.0);
  } else {
    picarxObject->stop();
  }
  for (agent::uint32 sample = 0U;
       sample < configurationValue.maximumLineRecoverySamples; ++sample) {
    const agent::boolean operationRequested =
        callbacks.vision.continueOperation(callbacks.visionContext);
    if (operationRequested == false) {
      picarxObject->stop();
      return false;
    }
    readings = picarxObject->grayscaleData();
    status = picarxObject->lineStatus(readings);
    if (status[0U] || status[1U] || status[2U]) {
      static_cast<void>(wait(1U));
      return true;
    }
  }
  picarxObject->stop();
  return true;
}

agent::boolean XWalkAppControl::applyObstacleAvoidance() {
  const agent::float64 distanceCm = picarxObject->distance();
  if (distanceCm <= 0.0) {
    picarxObject->stop();
    return true;
  }
  if (distanceCm >= 40.0) {
    picarxObject->setDirectionServoAngle(0.0);
    picarxObject->forward(configurationValue.obstacleSpeedPercent);
  } else if (distanceCm >= 20.0) {
    picarxObject->setDirectionServoAngle(30.0);
    picarxObject->forward(configurationValue.obstacleSpeedPercent);
    return wait(100U);
  } else {
    picarxObject->setDirectionServoAngle(-30.0);
    picarxObject->backward(configurationValue.obstacleSpeedPercent);
    return wait(500U);
  }
  return true;
}

void XWalkAppControl::applyJoystick(const XWalkAppControlInput &input) {
  if (!input.driveJoystickAvailable) {
    return;
  }
  const agent::float64 direction =
      std::clamp(input.driveX, -100.0, 100.0) * 0.3;
  const agent::float64 signedSpeed = std::clamp(input.driveY, -100.0, 100.0);
  picarxObject->setDirectionServoAngle(direction);
  speedPercentValue = (signedSpeed < 0.0) ? -signedSpeed : signedSpeed;
  if (signedSpeed > 0.0) {
    picarxObject->forward(speedPercentValue);
  } else if (signedSpeed < 0.0) {
    picarxObject->backward(speedPercentValue);
  } else {
    picarxObject->stop();
  }
}

XWalkAppControlResult XWalkAppControl::step() {
  if (startedValue == false) {
    XWALK_RPIAGENT_ERROR(XWALK_LOGIC,
                         "App control must be started before stepping");
  }
  XWalkAppControlResult result{};
  const agent::boolean operationRequested =
      callbacks.vision.continueOperation(callbacks.visionContext);
  if (operationRequested == false) {
    picarxObject->stop();
    result.event = XWalkAppControlEvent::Cancelled;
    return result;
  }

  result.telemetry.speedPercent = speedPercentValue;
  result.telemetry.grayscale = picarxObject->grayscaleData();
  result.telemetry.distanceCm = picarxObject->distance();
  result.telemetry.videoUrl = configurationValue.videoUrl;
  callbacks.publish(callbacks.transportContext, result.telemetry);
  const XWalkAppControlInput input = callbacks.poll(callbacks.transportContext);

  result.hornRequested = input.hornRequested;
  if (input.hornRequested) {
    result.event = XWalkAppControlEvent::HornRequested;
  }
  const agent::boolean spokenCommandAvailable =
      static_cast<agent::boolean>(!input.spokenCommand.empty());
  if (spokenCommandAvailable) {
    const agent::boolean voiceApplied = applyVoice(input.spokenCommand);
    if (voiceApplied == false) {
      result.event = XWalkAppControlEvent::Cancelled;
      return result;
    }
    result.event = XWalkAppControlEvent::VoiceMotion;
  }

  if (input.lineTrackingEnabled) {
    speedPercentValue = configurationValue.lineTrackingSpeedPercent;
    const agent::boolean lineTrackingApplied = applyLineTracking();
    if (lineTrackingApplied == false) {
      result.event = XWalkAppControlEvent::Cancelled;
      return result;
    }
    result.event = XWalkAppControlEvent::LineTracking;
  } else if (input.obstacleAvoidanceEnabled) {
    speedPercentValue = configurationValue.obstacleSpeedPercent;
    const agent::boolean obstacleAvoidanceApplied = applyObstacleAvoidance();
    if (obstacleAvoidanceApplied == false) {
      picarxObject->stop();
      result.event = XWalkAppControlEvent::Cancelled;
      return result;
    }
    result.event = XWalkAppControlEvent::ObstacleAvoidance;
  } else if (input.driveJoystickAvailable) {
    applyJoystick(input);
    result.event = XWalkAppControlEvent::JoystickMotion;
  }

  if (input.cameraJoystickAvailable) {
    picarxObject->setCameraPanAngle(
        std::clamp(input.cameraPanDegrees, -90.0, 90.0));
    picarxObject->setCameraTiltAngle(
        std::clamp(input.cameraTiltDegrees, -35.0, 65.0));
  }
  if (input.colorDetectionEnabled != lastColorEnabled) {
    callbacks.vision.setColor(callbacks.visionContext,
                              input.colorDetectionEnabled
                                  ? XWalkComputerVisionColor::Red
                                  : XWalkComputerVisionColor::Close);
    lastColorEnabled = input.colorDetectionEnabled;
  }
  if (input.faceDetectionEnabled != lastFaceEnabled) {
    callbacks.vision.setFace(callbacks.visionContext,
                             input.faceDetectionEnabled);
    lastFaceEnabled = input.faceDetectionEnabled;
  }
  if (input.objectDetectionEnabled != lastObjectEnabled) {
    lastObjectEnabled = input.objectDetectionEnabled;
    result.objectDetectionWarning = true;
    result.event = XWalkAppControlEvent::ObjectDetectionUnsupported;
  }
  const agent::boolean delayCompleted = wait(configurationValue.sampleDelayMs);
  if (delayCompleted == false) {
    picarxObject->stop();
    result.event = XWalkAppControlEvent::Cancelled;
  }
  return result;
}

} /* namespace xwalk::agent */
