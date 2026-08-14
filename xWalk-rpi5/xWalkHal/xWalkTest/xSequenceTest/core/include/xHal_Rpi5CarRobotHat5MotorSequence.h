/******************************************************************************
 * @file        xHal_Rpi5CarRobotHat5MotorSequence.h
 * @brief       Declares the bounded Robot HAT v5 four-motor sequence.
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

#ifndef XHAL_RPI5CAR_ROBOTHAT5_MOTOR_SEQUENCE_H
#define XHAL_RPI5CAR_ROBOTHAT5_MOTOR_SEQUENCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMotor.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Maximum bounded repetitions accepted by the physical sequence. */
#define XHAL_RPI5CAR_ROBOTHAT5_MOTOR_MAX_CYCLES 100U

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::test
{

/** @brief Wait operation injected for host and hardware execution. */
using motorsequencewaitcallback = void (*)(contextpointer context,
    uint32 durationMilliseconds);

/** @brief Alternates four dual-PWM motors and guarantees a final stop attempt. */
class XWalkRobotHat5MotorSequence
{
private:

    /** @brief Caller-owned motors corresponding to PWM pairs 12 through 19. */
    fixedarray<XWalkMotor*, 4U> motorObjects;
    /** @brief Non-owning context forwarded to the wait callback. */
    contextpointer waitContext;
    /** @brief Non-null bounded wait operation. */
    motorsequencewaitcallback waitCallback;

    /** @brief Applies one signed speed to all four motors. */
    void setAll(float64 speedPercent);
    /** @brief Makes an independent non-throwing stop attempt on every motor. */
    void stopAllSafely() noexcept;

public:

    /**
     * @brief Binds four caller-owned motors and one wait operation.
     *
     * @throws std::invalid_argument
     * If `wait` is null.
     */
    XWalkRobotHat5MotorSequence(XWalkMotor& firstMotor,
        XWalkMotor& secondMotor, XWalkMotor& thirdMotor,
        XWalkMotor& fourthMotor, contextpointer context,
        motorsequencewaitcallback wait);

    XWalkRobotHat5MotorSequence(const XWalkRobotHat5MotorSequence&) = delete;
    XWalkRobotHat5MotorSequence(XWalkRobotHat5MotorSequence&&) = delete;
    XWalkRobotHat5MotorSequence& operator=(
        const XWalkRobotHat5MotorSequence&) = delete;
    XWalkRobotHat5MotorSequence& operator=(
        XWalkRobotHat5MotorSequence&&) = delete;

    /**
     * @brief Runs the requested number of reverse/forward/stop cycles.
     *
     * @param[in] cycleCount
     * Inclusive cycle count from one through 100.
     *
     * @throws std::out_of_range
     * If `cycleCount` is outside its supported range.
     */
    void run(uint32 cycleCount);
};

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_ROBOTHAT5_MOTOR_SEQUENCE_H */
