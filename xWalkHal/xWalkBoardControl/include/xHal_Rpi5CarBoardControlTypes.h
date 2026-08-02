/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlTypes.h
 * @brief       Declares xWalk board-control callback types.
 *
 * @details
 * Defines the non-owning callback contract used to prime speaker output after
 * its hardware power-control line is enabled.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl
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

#ifndef XHAL_RPI5CAR_BOARD_CONTROL_TYPES_H
#define XHAL_RPI5CAR_BOARD_CONTROL_TYPES_H

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
 * @brief Callback that primes speaker output after speaker power is enabled.
 *
 * @param[in,out] context
 * Nullable non-owning application context supplied during construction. Null
 * is permitted only when the callback implementation supports it.
 *
 * @param[in] durationMs
 * Required priming duration in milliseconds.
 *
 * @pre
 * The context satisfies the callback and remains valid for the controller's
 * complete lifetime.
 */
using boardspeakerprimecallback = void (*)(contextpointer context, uint32 durationMs);

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_BOARD_CONTROL_TYPES_H */
