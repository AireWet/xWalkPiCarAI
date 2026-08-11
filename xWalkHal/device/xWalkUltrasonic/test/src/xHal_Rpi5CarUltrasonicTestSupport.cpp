/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicTestSupport.cpp
 * @brief       Implements reusable ultrasonic GPIO host-test support.
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarUltrasonicTestSupport.h"
namespace xwalk::hal::test::ultrasonic
{
void configure(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue)
{
    static_cast<void>(initialValue);
    TestBackend& backend = *static_cast<TestBackend*>(context);
    if (pin == 27U)
    {
        backend.triggerMode = mode;
    }
    if (pin == 22U)
    {
        backend.echoMode = mode;
        backend.echoPull = pull;
    }
}

boolean read(contextpointer context, uint8 pin)
{
    static_cast<void>(pin);
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.echoReadCount;
    EchoBehavior activeBehavior = backend.behavior;
    if ((activeBehavior == EchoBehavior::TimeoutThenPulse) && (backend.triggerCount > 1U))
    {
        activeBehavior = EchoBehavior::Pulse;
    }
    if ((activeBehavior == EchoBehavior::Timeout) ||
        (activeBehavior == EchoBehavior::TimeoutThenPulse))
    {
        return false;
    }
    if (activeBehavior == EchoBehavior::Invalid)
    {
        return backend.echoReadCount == 1U;
    }
    if (backend.echoReadCount == 3U)
    {
        common::sleepMicroseconds(backend.pulseDelayMicroseconds);
    }
    return (backend.echoReadCount == 2U) || (backend.echoReadCount == 3U);
}

void write(contextpointer context, uint8 pin, boolean value)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    if (pin == 27U)
    {
        backend.triggerLevels.push_back(value ? 1U : 0U);
        if (value)
        {
            ++backend.triggerCount;
            backend.echoReadCount = 0U;
        }
    }
}

void interrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(edge);
    static_cast<void>(debounceMs);
    static_cast<void>(handlerContext);
    static_cast<void>(handler);
}

void cancelInterrupt(contextpointer context, uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

XWalkGpioCallbacks callbacks()
{
    return {&configure, &read, &write, &interrupt, &cancelInterrupt};
}
} /* namespace xwalk::hal::test::ultrasonic */
