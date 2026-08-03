/******************************************************************************
 * @file        xHal_Rpi5CarMotorSequence.cpp
 * @brief       Implements the bounded two-motor Robot HAT sequence.
 *
 * @details
 * Reproduces motor_test.py with bounded repetition and cleanup on normal and
 * exceptional completion.
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

#include "xHal_Rpi5CarMotorSequence.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

XWalkMotorSequence::XWalkMotorSequence(XWalkMotor& firstMotor,
    XWalkMotor& secondMotor, contextpointer context,
    motortestwaitcallback wait)
    : motorObjects{{&firstMotor, &secondMotor}}, waitContext(context),
      waitCallback(wait)
{
    if (waitCallback == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Motor-sequence wait callback must not be null");
    }
}

void XWalkMotorSequence::setBoth(float64 speedPercent)
{
    for (XWalkMotor* const motor : motorObjects)
    {
        motor->setSpeed(speedPercent);
    }
}

void XWalkMotorSequence::stopBothSafely() noexcept
{
    for (XWalkMotor* const motor : motorObjects)
    {
        try
        {
            motor->setSpeed(0.0);
        }
        catch (...)
        {
            static_cast<void>(motor->stopSafely());
        }
    }
}

void XWalkMotorSequence::run(uint32 cycleCount)
{
    if ((cycleCount == 0U) ||
        (cycleCount > XHAL_RPI5CAR_MOTOR_SEQUENCE_MAX_CYCLES))
    {
        XHAL_THROW_OUT_OF_RANGE("Motor-sequence cycles must be from 1 to 100");
    }

    try
    {
        for (uint32 cycle = 0U; cycle < cycleCount; ++cycle)
        {
            setBoth(-50.0);
            waitCallback(waitContext, 1'000U);
            setBoth(50.0);
            waitCallback(waitContext, 1'000U);
            setBoth(0.0);
        }
    }
    catch (...)
    {
        stopBothSafely();
        try
        {
            waitCallback(waitContext, 100U);
        }
        catch (...)
        {
            /* Preserve the original sequence failure after the cleanup attempt. */
        }
        throw;
    }

    stopBothSafely();
    waitCallback(waitContext, 100U);
}

} /* namespace xwalk::hal::test */
