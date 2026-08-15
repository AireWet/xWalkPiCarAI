/******************************************************************************
 * @file        xHal_Rpi5CarServoTestI2cLifecycle.cpp
 * @brief       Implements lifecycle operations for the Servo I2C recorder.
 *
 * @details
 * Provides default construction and destruction for recorder-owned containers.
 *
 * @project     xWalk Firmware
 * @module      xWalkServo Host Test
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

#include "xHal_Rpi5CarServoTestI2c.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs an empty in-memory I2C recorder.
     *
     * @post
     * The write count is zero.
     */
    XWalkServoTestI2c::XWalkServoTestI2c() = default;

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /** @brief Destroys the recorder and its stored payloads. */
    XWalkServoTestI2c::~XWalkServoTestI2c() = default;

} /* namespace xwalk::hal::test */
