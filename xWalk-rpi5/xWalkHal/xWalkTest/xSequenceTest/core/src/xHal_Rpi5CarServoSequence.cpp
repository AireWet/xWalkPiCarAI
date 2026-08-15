/******************************************************************************
 * @file        xHal_Rpi5CarServoSequence.cpp
 * @brief       Implements the bounded 12-channel Robot HAT servo sequence.
 *
 * @details
 * Preserves the upstream channel order, angles, and per-command delay while
 * replacing its continuous loop with an explicit bounded cycle count.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarServoSequence.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

    /**
     * @brief Binds the ordered servo set and wait operation.
     *
     * @param[in] servos
     * Non-null servo pointers ordered by PWM channel zero through 11.
     *
     * @param[in,out] context
     * Non-owning context forwarded to `wait`.
     *
     * @param[in] wait
     * Non-null wait callback receiving durations in milliseconds.
     *
     * @throws std::invalid_argument
     * If `wait` or a servo pointer is null.
     */
    XWalkServoSequence::XWalkServoSequence(const servosequencearray& servos,
                                           contextpointer context,
                                           servosequencewaitcallback wait)
        : servoObjects(servos), waitContext(context), waitCallback(wait)
    {
        if (waitCallback == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Servo-sequence wait callback must not be null");
        }
        for (const XWalkServo* const servo : servoObjects)
        {
            if (servo == nullptr)
            {
                XWALK_HAL_ERROR(XWALK_INVAL, "Servo-sequence pointers must not be null");
            }
        }
    }

    /**
     * @brief Runs bounded negative and positive sweeps across all servos.
     *
     * @param[in] cycleCount
     * Complete sweep count in the inclusive range one through 100.
     *
     * @post
     * On normal completion, every servo's last requested angle is 20 degrees.
     *
     * @throws std::out_of_range
     * If `cycleCount` is outside its supported range.
     */
    void XWalkServoSequence::run(uint32 cycleCount)
    {
        if ((cycleCount == 0U) || (cycleCount > XHAL_RPI5CAR_SERVO_SEQUENCE_MAX_CYCLES))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Servo-sequence cycles must be from 1 to 100");
        }

        for (uint32 cycle = 0U; cycle < cycleCount; ++cycle)
        {
            for (XWalkServo* const servo : servoObjects)
            {
                servo->setAngle(-20.0);
                waitCallback(waitContext, 100U);
            }
            for (XWalkServo* const servo : servoObjects)
            {
                servo->setAngle(20.0);
                waitCallback(waitContext, 100U);
            }
        }
    }

} /* namespace xwalk::hal::test */
