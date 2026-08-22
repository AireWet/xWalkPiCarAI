/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonTestSupport.cpp
 * @brief       Implements reusable user-button host-test support.
 * @project     xWalk Firmware
 * @module      xWalkUserButton Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarUserButtonTestSupport.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::test::userbutton
{
    void configure(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
        static_cast<void>(initialValue);
    }

    boolean read(contextpointer context, uint8 pin)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        static_cast<void>(pin);
        const boolean readsFail = backend.failReads.load();
        if (readsFail)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Simulated user-button GPIO failure");
        }
        return backend.inputLevel.load();
    }

    void write(contextpointer context, uint8 pin, boolean value)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(value);
    }

    void registerInterrupt(contextpointer context,
                           uint8 pin,
                           XWalkGpioEdge edge,
                           uint32 debounceMs,
                           contextpointer handlerContext,
                           gpiointerrupthandler handler)
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

    XWalkGpioCallbacks gpioCallbacks()
    {
        return {&configure, &read, &write, &registerInterrupt, &cancelInterrupt};
    }

    void countClick(contextpointer context)
    {
        ++static_cast<EventCounts*>(context)->clicks;
    }
    void countPress(contextpointer context)
    {
        ++static_cast<EventCounts*>(context)->presses;
    }
    void countRelease(contextpointer context)
    {
        ++static_cast<EventCounts*>(context)->releases;
    }
    void countLongPress(contextpointer context)
    {
        ++static_cast<EventCounts*>(context)->longPresses;
    }
    void countLongRelease(contextpointer context)
    {
        ++static_cast<EventCounts*>(context)->longReleases;
    }
    void countState(contextpointer context, boolean pressed)
    {
        EventCounts& counts = *static_cast<EventCounts*>(context);
        if (pressed)
        {
            ++counts.pressedStates;
        }
        else
        {
            ++counts.releasedStates;
        }
    }
} /* namespace xwalk::hal::test::userbutton */
