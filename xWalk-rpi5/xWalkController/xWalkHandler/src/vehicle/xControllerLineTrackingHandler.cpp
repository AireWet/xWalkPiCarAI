/******************************************************************************
 * @file        xControllerLineTrackingHandler.cpp
 * @brief       Implements the LineTrackingHandler command responsibility.
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

#include "xControllerParsing.h"

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
 * @brief Executes foreground line-tracking start or immediate stop.
 * @param[in] request Validated lifecycle action.
 * @return Zero after tracking or stopping completes; three when the coordinator
 * is unavailable.
 */
::ctrl::int32 XWalkController::XWALK_handlerLineTracking(
    const XWalkLifecycleRequest &request) {
  if (lineTrackingObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Line-tracking backend unavailable");
    return 3;
  }
  if (request.action == XWalkLifecycleAction::Stop) {
    lineTrackingObject->finish();
    XWALK_CTRL_TRACE_UID0(CTRL .029, "Line tracking stopped");
    return 0;
  }

  XWALK_CTRL_TRACE_UID0(CTRL .030,
                        "Line tracking started; press Ctrl+C to stop");
  const ::ctrl::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const ::ctrl::boolean operationAllowed =
        static_cast<::ctrl::boolean>(operationMayContinue());
    if (operationAllowed == false) {
      break;
    }
    const agent::XWalkLineTrackingResult result = lineTrackingObject->step();
    const ::ctrl::string prefix =
        result.recoveryAttempted ? "outHandle gm_val_list: " : "gm_val_list: ";
    XWALK_CTRL_TRACE_UID3(
        CTRL .031, "%s%s, %s", prefix.c_str(),
        XWALK_FORMAT_VALUES(result.readings).c_str(),
        XWALK_FORMAT_LINE_TRACKING_STATE(result.state).c_str());
  }
  lineTrackingObject->finish();
  XWALK_CTRL_TRACE_UID0(CTRL .032, "Line tracking stopped");
  return 0;
}

} /* namespace xwalk::ctrl */
