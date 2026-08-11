/******************************************************************************
 * @file        xControllerVoicePromptCarHandler.cpp
 * @brief       Implements the VoicePromptCarHandler command responsibility.
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

/** @brief Executes one spoken movement-demonstration command. */
::ctrl::int32 XWalkController::XWALK_handlerVoicePromptCar(
    const XWalkLifecycleRequest &request) {
  if (voicePromptCarObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Voice-prompt-car backend unavailable");
    return 3;
  }
  if (request.action == XWalkLifecycleAction::Stop) {
    voicePromptCarObject->stop();
    XWALK_CTRL_TRACE_UID0(CTRL .085, "Voice-prompt car stopped");
    return 0;
  }
  return voicePromptCarObject->run();
}

} /* namespace xwalk::ctrl */
