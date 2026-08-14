/******************************************************************************
 * @file        xAgent_Rpi5CarFaceTracking.cpp
 * @brief       Implements one bounded face-centering iteration.
 * @project     xWalk Firmware
 * @module      xWalkFaceTracking
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarFaceTracking.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::agent {

agent::float64
XWalkFaceTracking::constrainAngle(agent::float64 angleDegrees) const noexcept {
  if (angleDegrees > configurationValue.maximumAngleDegrees) {
    return configurationValue.maximumAngleDegrees;
  }
  if (angleDegrees < -configurationValue.maximumAngleDegrees) {
    return -configurationValue.maximumAngleDegrees;
  }
  return angleDegrees;
}

XWalkFaceTrackingResult XWalkFaceTracking::step() {
  if (!startedValue) {
    XWALK_RPIAGENT_ERROR(XWALK_LOGIC,
                         "Face tracking must be started before stepping");
  }

  XWalkFaceTrackingResult result{};
  const agent::boolean operationRequested =
      callbacks.continueOperation(callbackContext);
  if (operationRequested == false) {
    picarxObject->stop();
    result.state = XWalkFaceTrackingState::Cancelled;
    return result;
  }

  const XWalkComputerVisionObservation observation =
      callbacks.observe(callbackContext);
  result.face = observation.face;
  if (result.face.count > 0U) {
    const agent::float64 halfSpan =
        configurationValue.correctionSpanDegrees / 2.0;
    const agent::float64 horizontalCorrection =
        (static_cast<agent::float64>(result.face.centerX) *
         configurationValue.correctionSpanDegrees /
         static_cast<agent::float64>(configurationValue.frameWidthPixels)) -
        halfSpan;
    const agent::float64 verticalCorrection =
        halfSpan -
        (static_cast<agent::float64>(result.face.centerY) *
         configurationValue.correctionSpanDegrees /
         static_cast<agent::float64>(configurationValue.frameHeightPixels));
    panAngleDegreesValue =
        constrainAngle(panAngleDegreesValue + horizontalCorrection);
    tiltAngleDegreesValue =
        constrainAngle(tiltAngleDegreesValue + verticalCorrection);
    picarxObject->setCameraPanAngle(panAngleDegreesValue);
    picarxObject->setCameraTiltAngle(tiltAngleDegreesValue);
    result.state = XWalkFaceTrackingState::Tracking;
  }
  result.panAngleDegrees = panAngleDegreesValue;
  result.tiltAngleDegrees = tiltAngleDegreesValue;
  const agent::boolean delayCompleted = wait(configurationValue.sampleDelayMs);
  if (delayCompleted == false) {
    picarxObject->stop();
    result.state = XWalkFaceTrackingState::Cancelled;
  }
  return result;
}

} /* namespace xwalk::agent */
