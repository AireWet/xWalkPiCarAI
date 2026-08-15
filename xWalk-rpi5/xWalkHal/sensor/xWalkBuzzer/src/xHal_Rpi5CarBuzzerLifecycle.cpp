/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerLifecycle.cpp
 * @brief       Implements buzzer construction and destruction.
 *
 * @details
 * Binds exactly one caller-owned PWM or GPIO dependency and places the buzzer
 * into its inactive state during construction.
 *
 * @project     xWalk Firmware
 * @module      xWalkBuzzer
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

#include "xHal_Rpi5CarBuzzer.h"

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
     * @brief Constructs a passive buzzer controller.
     *
     * @param[in] pwm
     * Non-owning PWM output used for frequency and duty-cycle control.
     *
     * @pre
     * `pwm` outlives this controller.
     *
     * @post
     * The PWM duty cycle is zero percent and the buzzer is inactive.
     */
    XWalkBuzzer::XWalkBuzzer(XWalkPwm& pwm) : pwmObject(&pwm)
    {
        off();
        XWALK_HAL_TRACE_UID0(RPI .275, "Passive PWM buzzer constructed in the inactive state");
    }

    /**
     * @brief Constructs an active buzzer controller.
     *
     * @param[in] gpio
     * Non-owning GPIO output used for logical activation.
     *
     * @pre
     * `gpio` outlives this controller.
     *
     * @post
     * The GPIO output is logically inactive.
     */
    XWalkBuzzer::XWalkBuzzer(XWalkGpio& gpio) : gpioObject(&gpio)
    {
        off();
        XWALK_HAL_TRACE_UID0(RPI .276, "Active GPIO buzzer constructed in the inactive state");
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Destroys the buzzer controller.
     *
     * @note
     * The selected dependency pointer is non-owning and is not released. The
     * destructor does not change the physical output state.
     */
    XWalkBuzzer::~XWalkBuzzer() = default;

} /* namespace xwalk::hal */
