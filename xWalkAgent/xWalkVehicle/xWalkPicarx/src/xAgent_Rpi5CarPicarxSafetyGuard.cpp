/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxSafetyGuard.cpp
 * @brief       Implements scope-bound PiCar-X emergency shutdown.
 *
 * @details
 * Binds one caller-owned coordinator and invokes its non-throwing emergency
 * stop whenever the command-level guard leaves scope.
 *
 * @project     xWalk Firmware
 * @module      xWalkPicarx
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

#include "xAgent_Rpi5CarPicarxSafetyGuard.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs an armed safety guard around one caller-owned coordinator.
 * @param[in] picarx PiCar-X coordinator that must outlive this guard.
 */
XWalkPicarxSafetyGuard::XWalkPicarxSafetyGuard(XWalkPicarx& picarx) noexcept
    : picarxObject(&picarx)
{
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Latches actuator suppression and independently attempts to stop both motors.
 * @post The observed PiCar-X emergency-stop state is latched.
 */
XWalkPicarxSafetyGuard::~XWalkPicarxSafetyGuard() noexcept
{
    static_cast<void>(picarxObject->emergencyStop());
}

} /* namespace xwalk::agent */
