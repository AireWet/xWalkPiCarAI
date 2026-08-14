/******************************************************************************
 * @file        xControllerTreasureHuntHandler.cpp
 * @brief       Implements the TreasureHuntHandler command responsibility.
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
 * @brief Runs the interactive color game ported from `20.treasure_hunt.py`.
 * @param[in] request Validated empty request.
 * @return Zero after quit or cancellation, two on camera startup failure, or
 * three without an Agent.
 * @warning Movement keys drive the physical car at 80-percent requested power.
 */
::ctrl::int32 XWalkController::XWALK_handlerTreasureHunt(
    const XWalkNoArgumentRequest &request) {
  static_cast<void>(request);
  if (treasureHuntObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Treasure-hunt backend unavailable");
    return 3;
  }
  const ::ctrl::boolean started = treasureHuntObject->start();
  if (started == false) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION,
                     "Treasure-hunt camera could not be started");
    return 2;
  }

  XWALK_CTRL_TRACE_UID0(CTRL .057, "Press keys to control PiCar-X:");
  XWALK_CTRL_TRACE_UID0(CTRL .058,
                        "w: forward  a: turn left  s: backward  d: turn right");
  XWALK_CTRL_TRACE_UID0(CTRL .059, "space: repeat target  quit: exit");
  XWALK_CTRL_TRACE_UID0(CTRL .060, "[SAY] Game start!");
  XWALK_CTRL_TRACE_UID1(
      CTRL .061, "[SAY] Look for %s!",
      agent::XWalkTreasureHunt::colorName(treasureHuntObject->targetColor())
          .c_str());
  const ::ctrl::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const ::ctrl::boolean operationAllowed =
        static_cast<::ctrl::boolean>(operationMayContinue());
    if (operationAllowed == false) {
      break;
    }
    const ::ctrl::string key = input("treasure> ");
    const agent::XWalkTreasureHuntResult result = treasureHuntObject->step(key);
    if (result.targetFound) {
      XWALK_CTRL_TRACE_UID0(CTRL .062, "[SAY] Well done!");
      XWALK_CTRL_TRACE_UID1(
          CTRL .063, "[SAY] Look for %s!",
          agent::XWalkTreasureHunt::colorName(result.targetColor).c_str());
    }
    if (result.action == agent::XWalkTreasureHuntAction::TargetRepeated) {
      XWALK_CTRL_TRACE_UID1(
          CTRL .064, "[SAY] Look for %s!",
          agent::XWalkTreasureHunt::colorName(result.targetColor).c_str());
    }
    if (result.action == agent::XWalkTreasureHuntAction::Quit) {
      XWALK_CTRL_TRACE_UID0(CTRL .065, "[INFO] Quit requested.");
      break;
    }
    if (result.action == agent::XWalkTreasureHuntAction::Cancelled) {
      break;
    }
  }
  treasureHuntObject->finish();
  XWALK_CTRL_TRACE_UID0(CTRL .066, "[SAY] Goodbye!");
  return 0;
}

} /* namespace xwalk::ctrl */
