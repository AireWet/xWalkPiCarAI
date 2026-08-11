/******************************************************************************
 * @file        xControllerBullFightHandler.cpp
 * @brief       Implements the BullFightHandler command responsibility.
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

::ctrl::int32
XWalkController::XWALK_handlerBullFight(const XWalkLifecycleRequest &request) {
  if (bullFightObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Bull-fight backend unavailable");
    return 3;
  }
  if (request.action == XWalkLifecycleAction::Stop) {
    bullFightObject->finish();
    XWALK_CTRL_TRACE_UID0(CTRL .042, "Bull fight stopped");
    return 0;
  }
  const ::ctrl::boolean started = bullFightObject->start();
  if (started == false) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Bull-fight camera could not be started");
    return 2;
  }

  XWALK_CTRL_TRACE_UID0(CTRL .043, "Bull fight started; press Ctrl+C to stop");
  const ::ctrl::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const ::ctrl::boolean operationAllowed =
        static_cast<::ctrl::boolean>(operationMayContinue());
    if (operationAllowed == false) {
      break;
    }
    const ::ctrl::boolean bullFightObjectStepStateMatched =
        static_cast<::ctrl::boolean>(bullFightObject->step().state ==
                                     agent::XWalkBullFightState::Cancelled);
    if (bullFightObjectStepStateMatched) {
      break;
    }
  }
  bullFightObject->finish();
  XWALK_CTRL_TRACE_UID0(CTRL .044, "stop and exit");
  return 0;
}

} /* namespace xwalk::ctrl */
