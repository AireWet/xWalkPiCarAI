/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveTypes.h
 * @brief       Declares self-drive action-flow types and timing callbacks.
 *
 * @details
 * Defines the status vocabulary and injected delay boundary used by the
 * PiCar-X preset-action coordinator.
 *
 * @project     xWalk Firmware
 * @module      xWalkSelfDrive
 *
 * @author      Joxy John
 * @date        2026-07-31
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_SELF_DRIVE_TYPES_H
#define XAGENT_RPI5CAR_SELF_DRIVE_TYPES_H

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

/**
 * @enum XWalkSelfDriveStatus
 * @brief Selects the current preset-action worker state.
 */
enum class XWalkSelfDriveStatus : agent::uint8
{
    /**
     * @brief Leaves the worker idle while retaining its thread.
     */
    Standby,
    /**
     * @brief Runs the continuous thinking pose once after entering the state.
     */
    Think,
    /**
     * @brief Consumes queued action names in first-in, first-out order.
     */
    Actions
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Suspends preset-action execution for a bounded interval.
 *
 * @param[in,out] context
 * Non-owning application context that must outlive the self-drive coordinator.
 *
 * @param[in] durationMs
 * Requested delay in milliseconds.
 *
 * @return
 * `true` when the delay completed; otherwise `false` to report a worker failure.
 */
using selfdrivedelaycallback = agent::boolean (*)(agent::contextpointer context,
    agent::uint32 durationMs) noexcept;

/**
 * @brief Reports whether one preset action may perform another bounded step.
 *
 * @param[in,out] context
 * Non-owning application context that must outlive the self-drive coordinator.
 *
 * @return
 * `true` to continue; otherwise `false` to latch emergency actuator shutdown.
 */
using selfdrivecontinuecallback = agent::boolean (*)(agent::contextpointer context);

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_SELF_DRIVE_TYPES_H */
