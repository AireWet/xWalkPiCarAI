/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonLifecycle.cpp
 * @brief       Implements user-button construction and destruction.
 *
 * @details
 * Binds the caller-owned GPIO, initializes monotonic timing state, and ensures
 * that a joinable monitoring worker is stopped before destruction.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarUserButton.h"

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
 * @brief Constructs a stopped user-button monitor.
 *
 * @param[in] gpio
 * Non-owning GPIO configured as the active-low pull-up button input.
 *
 * @pre
 * `gpio` outlives this object and its monitoring worker.
 */
XWalkUserButton::XWalkUserButton(XWalkGpio& gpio):
    gpioObject(&gpio), pressedAtMicrosecondsValue(common::monotonicMicroseconds())
{
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Stops the monitoring worker without releasing the caller-owned GPIO.
 *
 */
XWalkUserButton::~XWalkUserButton()
{
    stopWorker();
}

} /* namespace xwalk::hal */
