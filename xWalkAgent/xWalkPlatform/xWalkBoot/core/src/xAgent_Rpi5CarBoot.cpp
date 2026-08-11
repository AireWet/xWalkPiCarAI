/******************************************************************************
 * @file        xAgent_Rpi5CarBoot.cpp
 * @brief       Implements the shared one-shot xWalkBoot lifecycle guard.
 *
 * @details
 * Validates one application callback and consumes the boot object's run state
 * before any host or Raspberry Pi implementation begins platform work.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot
 *
 * @author      Joxy John
 * @date        2026-08-02
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
#include "xAgent_Rpi5CarBoot.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent {

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates and consumes this object's one boot attempt.
 * @param[in] callback Non-null synchronous application callback.
 * @throws std::invalid_argument If `callback` is null.
 * @throws std::logic_error If this object already started once.
 */
void XWalkBoot::begin(bootapplicationcallback callback) {
  if (callback == nullptr) {
    XWALK_RPIAGENT_ERROR(XWALK_INVAL,
                         "xWalkBoot application callback is required");
  }
  if (started) {
    XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "xWalkBoot object has already started");
  }
  started = true;
}

} /* namespace xwalk::agent */
