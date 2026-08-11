/******************************************************************************
 * @file        xHal_Rpi5CarStderrGuard.cpp
 * @brief       Defines scope-bound standard-error suppression.
 *
 * @details
 * Validates injected redirect operations, begins suppression during
 * construction, and restores standard error during destruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarStderrGuard.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Begins standard-error suppression through an injected backend.
 *
 * @param[in,out] backendContext
 * Nullable non-owning context used by both callbacks.
 *
 * @param[in] redirect
 * Non-null callback that begins suppression and returns a restore token.
 *
 * @param[in] restore
 * Non-null callback used during destruction.
 */
XWalkStderrGuard::XWalkStderrGuard(contextpointer backendContext,
                                   utilityredirectcallback redirect,
                                   utilityrestorecallback restore)
    : backendContextPointer(backendContext), restoreCallback(restore),
      restoreToken(0) {
  validateCallbacks(redirect, restore);
  restoreToken = redirect(backendContextPointer);
  XWALK_HAL_TRACE_UID0(RPI .128, "Utility standard-error guard activated");
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Restores standard error without releasing backend ownership. */
XWalkStderrGuard::~XWalkStderrGuard() {
  restoreCallback(backendContextPointer, restoreToken);
  XWALK_HAL_TRACE_UID0(RPI .129,
                       "Utility standard-error guard restored output");
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates both operations before beginning suppression.
 *
 * @param[in] redirect
 * Callback that must be non-null.
 *
 * @param[in] restore
 * Callback that must be non-null.
 */
void XWalkStderrGuard::validateCallbacks(utilityredirectcallback redirect,
                                         utilityrestorecallback restore) {
  if ((redirect == nullptr) || (restore == nullptr)) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Standard-error redirect callbacks must not be null");
  }
}

} /* namespace xwalk::hal */
