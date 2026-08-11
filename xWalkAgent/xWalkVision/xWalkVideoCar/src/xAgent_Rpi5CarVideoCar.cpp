/******************************************************************************
 * @file        xAgent_Rpi5CarVideoCar.cpp
 * @brief       Implements interactive video-car key handling.
 * @project     xWalk Firmware
 * @module      xWalkVideoCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoCar.h"

#include "xHal_Rpi5CarTrace.h"
#include <cctype>

namespace xwalk::agent {

void XWalkVideoCar::applyMotion() {
  const agent::float64 speed = static_cast<agent::float64>(speedPercentValue);
  switch (motionValue) {
  case XWalkVideoCarMotion::Stop:
    picarxObject->stop();
    break;
  case XWalkVideoCarMotion::Forward:
    picarxObject->setDirectionServoAngle(0.0);
    picarxObject->forward(speed);
    break;
  case XWalkVideoCarMotion::Backward:
    picarxObject->setDirectionServoAngle(0.0);
    picarxObject->backward(speed);
    break;
  case XWalkVideoCarMotion::TurnLeft:
    picarxObject->setDirectionServoAngle(
        -configurationValue.steeringAngleDegrees);
    picarxObject->forward(speed);
    break;
  case XWalkVideoCarMotion::TurnRight:
    picarxObject->setDirectionServoAngle(
        configurationValue.steeringAngleDegrees);
    picarxObject->forward(speed);
    break;
  }
}

XWalkVideoCarResult
XWalkVideoCar::result(XWalkVideoCarEvent event,
                      const agent::string &photoPath) const {
  return {event, motionValue, speedPercentValue, photoPath};
}

XWalkVideoCarResult XWalkVideoCar::handleKey(const agent::string &key) {
  if (!startedValue) {
    XWALK_RPIAGENT_ERROR(XWALK_LOGIC,
                         "Video car must be started before handling keys");
  }
  const agent::boolean operationRequested =
      callbacks.continueOperation(callbackContext);
  if (operationRequested == false) {
    picarxObject->stop();
    return result(XWalkVideoCarEvent::Cancelled);
  }

  XWalkVideoCarEvent event = XWalkVideoCarEvent::Ignored;
  agent::string photoPath;
  const char command =
      (key.size() == 1U)
          ? static_cast<char>(std::tolower(static_cast<unsigned char>(key[0])))
          : '\0';
  switch (command) {
  case 'o':
    if (speedPercentValue <= (configurationValue.maximumSpeedPercent -
                              configurationValue.speedStepPercent)) {
      speedPercentValue += configurationValue.speedStepPercent;
    }
    event = XWalkVideoCarEvent::SpeedChanged;
    applyMotion();
    break;
  case 'p':
    if (speedPercentValue >= configurationValue.speedStepPercent) {
      speedPercentValue -= configurationValue.speedStepPercent;
    }
    if (speedPercentValue == 0U) {
      motionValue = XWalkVideoCarMotion::Stop;
    }
    event = XWalkVideoCarEvent::SpeedChanged;
    applyMotion();
    break;
  case 'w':
    if (speedPercentValue == 0U) {
      speedPercentValue = configurationValue.speedStepPercent;
    }
    if ((motionValue != XWalkVideoCarMotion::Forward) &&
        (speedPercentValue > configurationValue.directionChangeCapPercent)) {
      speedPercentValue = configurationValue.directionChangeCapPercent;
    }
    motionValue = XWalkVideoCarMotion::Forward;
    event = XWalkVideoCarEvent::MotionChanged;
    applyMotion();
    break;
  case 's':
    if (speedPercentValue == 0U) {
      speedPercentValue = configurationValue.speedStepPercent;
    }
    if ((motionValue != XWalkVideoCarMotion::Backward) &&
        (speedPercentValue > configurationValue.directionChangeCapPercent)) {
      speedPercentValue = configurationValue.directionChangeCapPercent;
    }
    motionValue = XWalkVideoCarMotion::Backward;
    event = XWalkVideoCarEvent::MotionChanged;
    applyMotion();
    break;
  case 'a':
  case 'd':
    if (speedPercentValue == 0U) {
      speedPercentValue = configurationValue.speedStepPercent;
    }
    motionValue = (command == 'a') ? XWalkVideoCarMotion::TurnLeft
                                   : XWalkVideoCarMotion::TurnRight;
    event = XWalkVideoCarEvent::MotionChanged;
    applyMotion();
    break;
  case 'f':
    motionValue = XWalkVideoCarMotion::Stop;
    event = XWalkVideoCarEvent::MotionChanged;
    applyMotion();
    break;
  case 't':
    photoPath = callbacks.capture(callbackContext);
    event = XWalkVideoCarEvent::PhotoCaptured;
    break;
  default:
    break;
  }

  const agent::boolean delayCompleted = wait(configurationValue.keyDelayMs);
  if (delayCompleted == false) {
    picarxObject->stop();
    event = XWalkVideoCarEvent::Cancelled;
  }
  return result(event, photoPath);
}

agent::string XWalkVideoCar::motionName(XWalkVideoCarMotion motion) {
  switch (motion) {
  case XWalkVideoCarMotion::Forward:
    return "forward";
  case XWalkVideoCarMotion::Backward:
    return "backward";
  case XWalkVideoCarMotion::TurnLeft:
    return "turn left";
  case XWalkVideoCarMotion::TurnRight:
    return "turn right";
  case XWalkVideoCarMotion::Stop:
  default:
    return "stop";
  }
}

} /* namespace xwalk::agent */
