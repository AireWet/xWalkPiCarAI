/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerTestSupport.cpp
 * @brief       Implements reusable xWalkBuzzer host-test support.
 * @project     xWalk Firmware
 * @module      xWalkBuzzer Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBuzzerTestSupport.h"
namespace xwalk::hal::test::buzzer
{
boolean probe(contextpointer context, uint8 address)
{
    static_cast<void>(context); static_cast<void>(address); return true;
}
void writeRegister(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data)
{
    static_cast<void>(address); static_cast<void>(reg); static_cast<void>(data);
    ++static_cast<TestBackend*>(context)->i2cWriteCount;
}
bytevector read(contextpointer context, uint8 address, size length)
{
    static_cast<void>(context); static_cast<void>(address); return bytevector(length, 0U);
}
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(pin); static_cast<void>(mode); static_cast<void>(pull);
    ++backend.gpioConfigureCount; backend.gpioValue = initialValue;
}
boolean readGpio(contextpointer context, uint8 pin)
{
    static_cast<void>(pin); return static_cast<TestBackend*>(context)->gpioValue;
}
void writeGpio(contextpointer context, uint8 pin, boolean value)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(pin); ++backend.gpioWriteCount; backend.gpioValue = value;
}
void registerInterrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler)
{
    static_cast<void>(context); static_cast<void>(pin); static_cast<void>(edge);
    static_cast<void>(debounceMs); static_cast<void>(handlerContext);
    static_cast<void>(handler);
}
void cancelInterrupt(contextpointer context, uint8 pin)
{
    static_cast<void>(context); static_cast<void>(pin);
}
XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
}
PassiveFixture::PassiveFixture():
    i2c(&backend, &probe, &writeRegister, &read),
    pwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState)
{
}
} /* namespace xwalk::hal::test::buzzer */
