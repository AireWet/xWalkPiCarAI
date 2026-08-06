/******************************************************************************
 * @file        xAgent_Rpi5CarKeyboardControlTypes.h
 * @brief       Declares keyboard-control callback and result types.
 *
 * @details
 * Defines the injected scheduling boundary and observable key-processing
 * outcome used by the keyboard-control Agent.
 *
 * @project     xWalk Firmware
 * @module      xWalkKeyboardControl
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_KEYBOARD_CONTROL_TYPES_H
#define XAGENT_RPI5CAR_KEYBOARD_CONTROL_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/** @brief Reports the outcome of processing one keyboard-control key. */
enum class XWalkKeyboardControlResult : agent::uint8
{
    /** @brief The key completed its source-compatible actuator pulse. */
    Handled,
    /** @brief The key is outside the supported `wsadikjl` set. */
    Ignored,
    /** @brief Cancellation interrupted the key before completion. */
    Cancelled
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Suspends keyboard-control execution for one bounded interval.
 * @param[in,out] context Non-owning context that outlives the Agent.
 * @param[in] durationMs Requested delay in milliseconds.
 */
using keyboardcontroldelaycallback = void (*)(agent::contextpointer context,
    agent::uint32 durationMs);

/**
 * @brief Reports whether keyboard-control execution may continue.
 * @param[in,out] context Non-owning context that outlives the Agent.
 * @return `true` to continue or `false` to request cleanup.
 */
using keyboardcontrolcontinuecallback = agent::boolean (*)(agent::contextpointer context);

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_KEYBOARD_CONTROL_TYPES_H */
