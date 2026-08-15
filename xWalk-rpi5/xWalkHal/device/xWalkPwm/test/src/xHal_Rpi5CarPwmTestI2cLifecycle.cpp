/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestI2cLifecycle.cpp
 * @brief       Implements lifecycle operations for the PWM I2C test double.
 *
 * @details
 * Provides construction and destruction behavior for the in-memory recording
 * backend. Callback-interface composition is performed by each test function.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Test
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

#include "xHal_Rpi5CarPwmTestI2c.h"

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
     * @brief Constructs an empty in-memory I2C recording backend.
     *
     * @post
     * All interaction records are empty and no simulated address is present.
     */
    XWalkPwmTestI2c::XWalkPwmTestI2c() = default;

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Destroys the test double and its recorded data.
     *
     * @pre
     * Any separately created `XWalkI2c` object whose context points here has
     * already been destroyed or will no longer be used.
     */
    XWalkPwmTestI2c::~XWalkPwmTestI2c() = default;

} /* namespace xwalk::hal::test */
