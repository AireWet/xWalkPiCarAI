/******************************************************************************
 * @file        xControllerGptCarHandler.cpp
 * @brief       Implements the GptCarHandler command responsibility.
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
 * @brief Executes the upstream GPT-car loop with optional source flags.
 * @param[in] request Validated lifecycle action and input-source flags.
 * @return Zero on completion or three when the selected backend is unavailable.
 */
::ctrl::int32
XWalkController::XWALK_handlerGptCar(const XWalkGptCarRequest &request) {
  if (gptCarObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "GPT-car backend unavailable");
    return 3;
  }
  if (request.action == XWalkLifecycleAction::Stop) {
    gptCarObject->stop();
    XWALK_CTRL_TRACE_UID0(CTRL .077, "GPT car stopped");
    return 0;
  }
  gptCarObject->configure(request.keyboardInput, request.withImage);
  return gptCarObject->run();
}

} /* namespace xwalk::ctrl */
