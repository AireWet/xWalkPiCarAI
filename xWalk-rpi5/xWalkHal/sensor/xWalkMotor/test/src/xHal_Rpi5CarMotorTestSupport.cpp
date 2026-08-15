/******************************************************************************
 * @file        xHal_Rpi5CarMotorTestSupport.cpp
 * @brief       Implements reusable xWalkMotor host-test support.
 * @project     xWalk Firmware
 * @module      xWalkMotor Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarMotorTestSupport.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::test::motor
{
    uint64 clockMilliseconds(contextpointer context)
    {
        return static_cast<FakeClock*>(context)->milliseconds;
    }
    void failThreadStart(contextpointer context)
    {
        static_cast<void>(context);
        XWALK_HAL_ERROR(XWALK_RUNTIME, "simulated watchdog thread-start failure");
    }
    boolean probe(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return false;
    }
    void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
        TestI2c& bus = *static_cast<TestI2c*>(context);
        ++bus.writeCount;
        for (const uint32 failingWrite : bus.failingWrites)
        {
            if (bus.writeCount == failingWrite)
            {
                XWALK_HAL_ERROR(XWALK_RUNTIME, "simulated motor PWM write failure");
            }
        }
    }
    boolean tryWriteRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data) noexcept
    {
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
        TestI2c& bus = *static_cast<TestI2c*>(context);
        ++bus.writeCount;
        for (const uint32 failingWrite : bus.failingWrites)
        {
            if (bus.writeCount == failingWrite)
            {
                return false;
            }
        }
        return true;
    }
    bytevector read(contextpointer context, uint8 address, size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return bytevector(length, 0U);
    }
    void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
        static_cast<void>(initialValue);
    }
    boolean readGpio(contextpointer context, uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<TestGpio*>(context)->value;
    }
    void writeGpio(contextpointer context, uint8 pin, boolean value)
    {
        static_cast<void>(pin);
        TestGpio& gpio = *static_cast<TestGpio*>(context);
        gpio.value = value;
        ++gpio.writeCount;
    }
    void interruptGpio(contextpointer context,
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
    void cancelGpioInterrupt(contextpointer context, uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }
    XWalkGpioCallbacks gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelGpioInterrupt};
    }
} /* namespace xwalk::hal::test::motor */
