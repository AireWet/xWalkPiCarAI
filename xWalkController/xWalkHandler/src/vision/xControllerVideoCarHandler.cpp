/******************************************************************************
 * @file        xControllerVideoCarHandler.cpp
 * @brief       Implements the VideoCarHandler command responsibility.
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
XWalkController::XWALK_handlerVideoCar(const XWalkNoArgumentRequest &request) {
  static_cast<void>(request);
  if (videoCarObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Video-car backend unavailable");
    return 3;
  }
  const ::ctrl::boolean started = videoCarObject->start();
  if (started == false) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Video-car camera could not be started");
    return 2;
  }

  XWALK_CTRL_TRACE_UID0(CTRL .067,
                        "Video-car keys: o/p speed; w/s forward/backward; "
                        "a/d left/right; f stop; t photo; x exit.");
  agent::XWalkVideoCarMotion motion = agent::XWalkVideoCarMotion::Stop;
  ::ctrl::uint32 speedPercent = 0U;
  const ::ctrl::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const ::ctrl::boolean operationAllowed =
        static_cast<::ctrl::boolean>(operationMayContinue());
    if (operationAllowed == false) {
      break;
    }
    XWALK_CTRL_TRACE_UID2(CTRL .068, "status: %s, speed: %u",
                          agent::XWalkVideoCar::motionName(motion).c_str(),
                          speedPercent);
    const ::ctrl::string key = input("video-car> ");
    if ((key == "x") || (key == "X") || (key == "exit") || (key == "quit") ||
        (key == "skip")) {
      break;
    }
    const agent::XWalkVideoCarResult result = videoCarObject->handleKey(key);
    motion = result.motion;
    speedPercent = result.speedPercent;
    if (result.event == agent::XWalkVideoCarEvent::PhotoCaptured) {
      XWALK_CTRL_TRACE_UID1(CTRL .069, "photo save as %s",
                            result.photoPath.c_str());
    }
    if (result.event == agent::XWalkVideoCarEvent::Cancelled) {
      break;
    }
  }
  videoCarObject->finish();
  XWALK_CTRL_TRACE_UID0(CTRL .070, "quit");
  return 0;
}

} /* namespace xwalk::ctrl */
