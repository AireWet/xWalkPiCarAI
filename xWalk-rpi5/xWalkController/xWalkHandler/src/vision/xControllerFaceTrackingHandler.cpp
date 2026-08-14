/******************************************************************************
 * @file        xControllerFaceTrackingHandler.cpp
 * @brief       Implements the FaceTrackingHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based
 *handler group.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-08-06
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

#include "xController.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl {

/******************************************************************************
 * Member function definitions
 ******************************************************************************/

/**
 * @brief Runs bounded camera-servo face tracking ported from
 * `8.stare_at_you.py`.
 * @param[in] request Validated lifecycle action.
 * @return Zero after cleanup, two on camera failure, or three without an Agent.
 */
::ctrl::int32 XWalkController::XWALK_handlerFaceTracking(
    const XWalkLifecycleRequest &request) {
  if (faceTrackingObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Face-tracking backend unavailable");
    return 3;
  }
  if (request.action == XWalkLifecycleAction::Stop) {
    faceTrackingObject->finish();
    XWALK_CTRL_TRACE_UID0(CTRL .054, "Face tracking stopped");
    return 0;
  }
  const ::ctrl::boolean started = faceTrackingObject->start();
  if (started == false) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION,
                     "Face-tracking camera could not be started");
    return 2;
  }

  XWALK_CTRL_TRACE_UID0(CTRL .055,
                        "Face tracking started; press Ctrl+C to stop");
  const ::ctrl::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const ::ctrl::boolean operationAllowed =
        static_cast<::ctrl::boolean>(operationMayContinue());
    if (operationAllowed == false) {
      break;
    }
    const ::ctrl::boolean faceTrackingObjectStepStateMatched =
        static_cast<::ctrl::boolean>(faceTrackingObject->step().state ==
                                     agent::XWalkFaceTrackingState::Cancelled);
    if (faceTrackingObjectStepStateMatched) {
      break;
    }
  }
  faceTrackingObject->finish();
  XWALK_CTRL_TRACE_UID0(CTRL .056, "stop and exit");
  return 0;
}

} /* namespace xwalk::ctrl */
