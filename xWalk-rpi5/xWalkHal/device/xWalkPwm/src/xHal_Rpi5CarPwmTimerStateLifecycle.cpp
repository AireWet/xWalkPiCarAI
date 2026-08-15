/******************************************************************************
 * @file        xHal_Rpi5CarPwmTimerStateLifecycle.cpp
 * @brief       Implements lifecycle operations for shared PWM timer state.
 *
 * @details
 * Initializes all seven timer periods to the safe non-zero value used before
 * explicit timer configuration and provides default destruction behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarPwmTimerState.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs shared timer state with non-zero initial periods.
     *
     * @post
     * All seven timer periods contain one timer-count unit.
     */
    XWalkPwmTimerState::XWalkPwmTimerState()
    {
        periods.fill(1U);
        XWALK_HAL_TRACE_UID0(RPI .166, "PWM shared timer state constructed");
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Destroys the shared timer state.
     *
     * @pre
     * All non-owning `pwmtimerstatepointer` observers have stopped using this
     * object.
     */
    XWalkPwmTimerState::~XWalkPwmTimerState() = default;

} /* namespace xwalk::hal */
