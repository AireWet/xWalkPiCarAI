/******************************************************************************
 * @file        xHal_Rpi5CarMotorHostStub.cpp
 * @brief       Implements the device-free Motor host stub.
 * @project     xWalk Firmware
 * @module      xWalkMotor Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarMotorHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    boolean XWalkMotorHostStub::probe(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return false;
    }
    void XWalkMotorHostStub::writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
        ++static_cast<XWalkMotorHostStub*>(context)->i2cWriteCountValue;
        XWALK_HAL_TRACE_UID0(RPI .252, "Motor host stub accepted an I2C register write");
    }
    boolean XWalkMotorHostStub::tryWriteRegister(contextpointer context,
                                                 uint8 address,
                                                 uint8 reg,
                                                 const bytevector& data) noexcept
    {
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
        ++static_cast<XWalkMotorHostStub*>(context)->i2cWriteCountValue;
        return true;
    }
    bytevector XWalkMotorHostStub::read(contextpointer context, uint8 address, size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return bytevector(length, 0U);
    }
    void XWalkMotorHostStub::configureGpio(
        contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
        static_cast<void>(initialValue);
    }
    boolean XWalkMotorHostStub::readGpio(contextpointer context, uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<XWalkMotorHostStub*>(context)->directionValue;
    }
    void XWalkMotorHostStub::writeGpio(contextpointer context, uint8 pin, boolean value)
    {
        static_cast<void>(pin);
        static_cast<XWalkMotorHostStub*>(context)->directionValue = value;
    }
    void XWalkMotorHostStub::interruptGpio(contextpointer context,
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
    void XWalkMotorHostStub::cancelGpioInterrupt(contextpointer context, uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }
    XWalkGpioCallbacks XWalkMotorHostStub::gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelGpioInterrupt};
    }
    uint32 XWalkMotorHostStub::i2cWriteCount() const noexcept
    {
        return i2cWriteCountValue;
    }
    boolean XWalkMotorHostStub::direction() const noexcept
    {
        return directionValue;
    }
} /* namespace xwalk::hal::sim */
