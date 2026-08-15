/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonHostStub.cpp
 * @brief       Implements the device-free UserButton GPIO host stub.
 * @project     xWalk Firmware
 * @module      xWalkUserButton Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarUserButtonHostStub.h"
namespace xwalk::hal::sim
{
    void XWalkUserButtonHostStub::configure(
        contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
        static_cast<void>(initialValue);
    }
    boolean XWalkUserButtonHostStub::read(contextpointer context, uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<XWalkUserButtonHostStub*>(context)->inputLevelValue.load();
    }
    void XWalkUserButtonHostStub::write(contextpointer context, uint8 pin, boolean value)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(value);
    }
    void XWalkUserButtonHostStub::interrupt(contextpointer context,
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
    void XWalkUserButtonHostStub::cancelInterrupt(contextpointer context, uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }
    void XWalkUserButtonHostStub::countClick(contextpointer context)
    {
        ++static_cast<XWalkUserButtonHostStub*>(context)->clickCountValue;
    }
    XWalkGpioCallbacks XWalkUserButtonHostStub::callbacks()
    {
        return {&configure, &read, &write, &interrupt, &cancelInterrupt};
    }
    void XWalkUserButtonHostStub::setPressed(boolean pressed) noexcept
    {
        inputLevelValue.store(pressed == false);
    }
    uint32 XWalkUserButtonHostStub::clickCount() const noexcept
    {
        return clickCountValue;
    }
} /* namespace xwalk::hal::sim */
