/******************************************************************************
 * @file        xControllerVoiceActiveCarHandler.cpp
 * @brief       Implements the VoiceActiveCarHandler command responsibility.
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
 * @brief Executes one sensor-aware voice-active-car command.
 * @param[in] request Validated lifecycle action.
 * @return Zero on completion or three when the selected backend is unavailable.
 */
::ctrl::int32 XWalkController::XWALK_handlerVoiceActiveCar(
    const XWalkLifecycleRequest &request) {
  if (voiceActiveCarObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Voice-active-car backend unavailable");
    return 3;
  }
  if (request.action == XWalkLifecycleAction::Stop) {
    voiceActiveCarObject->stop();
    XWALK_CTRL_TRACE_UID0(CTRL .082, "Voice-active car stopped");
    return 0;
  }
  return voiceActiveCarObject->run();
}

} /* namespace xwalk::ctrl */
