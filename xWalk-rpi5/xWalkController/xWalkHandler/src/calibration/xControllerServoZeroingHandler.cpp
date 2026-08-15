/******************************************************************************
 * @file        xControllerServoZeroingHandler.cpp
 * @brief       Implements the ServoZeroingHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based
 *handler group.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-08-06
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

#include "xController.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

    /******************************************************************************
     * Member function definitions
     ******************************************************************************/

    /**
     * @brief Runs the all-channel sequence ported from `servo_zeroing.py`.
     * @param[in] request Validated empty request.
     * @return Zero after the channels reach zero and cancellation ends the idle
     * loop.
     */
    ::ctrl::int32 XWalkController::XWALK_handlerServoZeroing(const XWalkNoArgumentRequest& request)
    {
        static_cast<void>(request);
        if (servoZeroingObject == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Servo-zeroing backend unavailable");
            return 3;
        }
        XWALK_CTRL_TRACE_UID0(CTRL .016, "Set servo to zero");
        static_cast<void>(servoZeroingObject->run());
        return 0;
    }

} /* namespace xwalk::ctrl */
