/******************************************************************************
 * @file        xControllerCliffDetectionHandler.cpp
 * @brief       Implements the CliffDetectionHandler command responsibility.
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
 * @brief Runs foreground cliff detection ported from `5.cliff_detection.py`.
 * @param[in] request Validated lifecycle action.
 * @return Zero after stop or cancellation, or three when the Agent is
 * unavailable.
 * @warning Danger samples continuously command physical reverse movement.
 */
::ctrl::int32 XWalkController::XWALK_handlerCliffDetection(
    const XWalkLifecycleRequest &request) {
  if (cliffDetectionObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Cliff-detection backend unavailable.");
    return 3;
  }
  if (request.action == XWalkLifecycleAction::Stop) {
    cliffDetectionObject->stop();
    XWALK_CTRL_TRACE_UID0(CTRL .025, "Cliff detection stopped.");
    return 0;
  }

  const ::ctrl::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const ::ctrl::boolean cliffDetectionObjectStepDifferent =
        static_cast<::ctrl::boolean>(
            cliffDetectionObject->step() !=
            agent::XWalkCliffDetectionResult::Cancelled);
    if (cliffDetectionObjectStepDifferent == false) {
      break;
    }
  }
  cliffDetectionObject->stop();
  XWALK_CTRL_TRACE_UID0(CTRL .026, "Cliff detection stopped.");
  return 0;
}

} /* namespace xwalk::ctrl */
