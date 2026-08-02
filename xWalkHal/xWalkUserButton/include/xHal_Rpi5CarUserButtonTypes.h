/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonTypes.h
 * @brief       Declares user-button callback types.
 *
 * @details
 * Defines context-based callbacks for button events without dynamic callback
 * allocation or ownership transfer.
 *
 * @project     xWalk Firmware
 * @module      xWalkUserButton
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_USER_BUTTON_TYPES_H
#define XHAL_RPI5CAR_USER_BUTTON_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Callback invoked for a user-button event without state data.
 *
 * @param[in,out] context
 * Non-owning callback context whose interpretation is callback-specific.
 */
using userbuttoncallback = void (*)(contextpointer context);

/**
 * @brief Callback invoked when user-button pressed state changes.
 *
 * @param[in,out] context
 * Non-owning callback context whose interpretation is callback-specific.
 *
 * @param[in] pressed
 * `true` for a press transition or `false` for a release transition.
 */
using userbuttonstatecallback = void (*)(contextpointer context, boolean pressed);

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_USER_BUTTON_TYPES_H */
