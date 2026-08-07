/******************************************************************************
 * @file        xHal_Rpi5CarRobotHat5MotorSequence.cpp
 * @brief       Implements the bounded Robot HAT v5 four-motor sequence.
 *
 * @details
 * Reproduces the upstream alternating motor commands with bounded repetition
 * and scope-equivalent cleanup for normal and exceptional completion.
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

#include "xHal_Rpi5CarRobotHat5MotorSequence.h"

#include <cstdio>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

XWalkRobotHat5MotorSequence::XWalkRobotHat5MotorSequence(
    XWalkMotor& firstMotor, XWalkMotor& secondMotor,
    XWalkMotor& thirdMotor, XWalkMotor& fourthMotor,
    contextpointer context, motorsequencewaitcallback wait)
    : motorObjects{{&firstMotor, &secondMotor, &thirdMotor, &fourthMotor}},
      waitContext(context), waitCallback(wait)
{
    if (waitCallback == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Motor-sequence wait callback must not be null");
    }
}

void XWalkRobotHat5MotorSequence::setAll(float64 speedPercent)
{
    for (XWalkMotor* const motor : motorObjects)
    {
        motor->setSpeed(speedPercent);
    }
}

void XWalkRobotHat5MotorSequence::stopAllSafely() noexcept
{
    for (XWalkMotor* const motor : motorObjects)
    {
        static_cast<void>(motor->stopSafely());
    }
}

void XWalkRobotHat5MotorSequence::run(uint32 cycleCount)
{
    if ((cycleCount == 0U) ||
        (cycleCount > XHAL_RPI5CAR_ROBOTHAT5_MOTOR_MAX_CYCLES))
    {
        XHAL_THROW_OUT_OF_RANGE("Robot HAT v5 motor cycles must be from 1 to 100");
    }

    try
    {
        for (uint32 cycle = 0U; cycle < cycleCount; ++cycle)
        {
            setAll(-50.0);
            waitCallback(waitContext, 1'000U);
            setAll(50.0);
            waitCallback(waitContext, 1'000U);
            setAll(0.0);
        }
    }
    catch (...)
    {
        stopAllSafely();
        try
        {
            waitCallback(waitContext, 100U);
        }
        catch (...)
        {
            stopAllSafely();
            static_cast<void>(std::fputs(
                "Robot HAT v5 motor-sequence cleanup delay failed\n", stderr));
        }
        throw;
    }

    stopAllSafely();
    waitCallback(waitContext, 100U);
}

} /* namespace xwalk::hal::test */
