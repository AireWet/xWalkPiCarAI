/******************************************************************************
 * @file        xHal_Rpi5CarServoSequence.h
 * @brief       Declares the bounded 12-channel Robot HAT servo sequence.
 *
 * @details
 * Alternates caller-owned servos on PWM channels zero through 11 between
 * negative and positive twenty-degree positions with injected timing.
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

#ifndef XHAL_RPI5CAR_SERVO_SEQUENCE_H
#define XHAL_RPI5CAR_SERVO_SEQUENCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServo.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Maximum bounded sweep count accepted by the physical sequence. */
#define XHAL_RPI5CAR_SERVO_SEQUENCE_MAX_CYCLES 100U

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Non-owning ordered servo pointers for PWM channels zero through 11. */
using servosequencearray = fixedarray<XWalkServo*, 12U>;

/**
 * @brief Wait operation injected for host and hardware execution.
 *
 * @param[in,out] context
 * Non-owning callback context whose nullability is implementation-defined.
 *
 * @param[in] durationMilliseconds
 * Requested wait duration in milliseconds.
 */
using servosequencewaitcallback = void (*)(contextpointer context,
    uint32 durationMilliseconds);

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @brief Alternates 12 servos through bounded negative and positive sweeps.
 *
 * @details
 * Stores only non-owning pointers and callback context. Every servo and the
 * callback implementation must outlive the sequence.
 */
class XWalkServoSequence
{
private:

    /** @brief Caller-owned servos ordered by PWM channel zero through 11. */
    servosequencearray servoObjects;
    /** @brief Non-owning context forwarded to the wait callback. */
    contextpointer waitContext;
    /** @brief Non-null wait operation used after every angle command. */
    servosequencewaitcallback waitCallback;

public:

    /**
     * @brief Binds the ordered servo set and wait operation.
     *
     * @param[in] servos
     * Non-null servo pointers ordered by PWM channel zero through 11. Every
     * servo must outlive the sequence.
     *
     * @param[in,out] context
     * Non-owning context forwarded to `wait`; nullability is callback-defined.
     *
     * @param[in] wait
     * Non-null wait callback receiving durations in milliseconds.
     *
     * @throws std::invalid_argument
     * If `wait` or a servo pointer is null.
     */
    XWalkServoSequence(const servosequencearray& servos,
        contextpointer context, servosequencewaitcallback wait);

    /** @brief Prevents copying of non-owning dependency bindings. */
    XWalkServoSequence(const XWalkServoSequence&) = delete;
    /** @brief Prevents moving of non-owning dependency bindings. */
    XWalkServoSequence(XWalkServoSequence&&) = delete;
    /** @brief Prevents copy assignment of non-owning dependency bindings. */
    XWalkServoSequence& operator=(const XWalkServoSequence&) = delete;
    /** @brief Prevents move assignment of non-owning dependency bindings. */
    XWalkServoSequence& operator=(XWalkServoSequence&&) = delete;

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
    void run(uint32 cycleCount);
};

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_SERVO_SEQUENCE_H */
