/******************************************************************************
 * @file        xControllerAppControlHandler.cpp
 * @brief       Implements the AppControlHandler command responsibility.
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
XWalkController::XWALK_handlerAppControl(const XWalkLifecycleRequest &request) {
  if (appControlObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "App-control backend unavailable");
    return 3;
  }
  if (request.action == XWalkLifecycleAction::Stop) {
    appControlObject->finish();
    XWALK_CTRL_TRACE_UID0(CTRL .017, "App control stopped");
    return 0;
  }
  const ::ctrl::boolean started = appControlObject->start();
  if (started == false) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION,
                     "App-control transport or camera could not be started");
    return 2;
  }

  XWALK_CTRL_TRACE_UID0(CTRL .018, "App control started; press Ctrl+C to stop");
  const ::ctrl::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const ::ctrl::boolean operationAllowed =
        static_cast<::ctrl::boolean>(operationMayContinue());
    if (operationAllowed == false) {
      break;
    }
    const agent::XWalkAppControlResult result = appControlObject->step();
    const XWalkSoundRequest hornRequest{
        XWalkSoundOperation::Play, "car-double-horn.wav", {}};
    if (result.hornRequested) {
      const ::ctrl::boolean hornPlayed =
          callbacks.sound(callbackContext, hornRequest);
      if (hornPlayed == false) {
        XWALK_CTRL_ERROR(XWALK_EXCEPTION,
                         "App-control horn backend unavailable");
      }
    }
    if (result.objectDetectionWarning) {
      XWALK_CTRL_WARNING(XWALK_SYSTEM,
                         "Object detection is not available for this build");
    }
    if (result.event == agent::XWalkAppControlEvent::Cancelled) {
      break;
    }
  }
  appControlObject->finish();
  XWALK_CTRL_TRACE_UID0(CTRL .020, "stop and exit");
  return 0;
}

} /* namespace xwalk::ctrl */
