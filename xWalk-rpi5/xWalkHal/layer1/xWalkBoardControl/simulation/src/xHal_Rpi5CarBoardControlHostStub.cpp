/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlHostStub.cpp
 * @brief       Implements device-free BoardControl simulation adapters.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBoardControlHostStub.h"
namespace xwalk::hal::sim
{
void XWalkBoardControlHostStub::configureGpio(contextpointer context, uint8 pin,
    XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
{
    static_cast<XWalkBoardControlHostStub*>(context)->gpioValueValue = initialValue;
    static_cast<void>(pin); static_cast<void>(mode); static_cast<void>(pull);
}
boolean XWalkBoardControlHostStub::readGpio(contextpointer context, uint8 pin)
{
    static_cast<void>(pin); return static_cast<XWalkBoardControlHostStub*>(context)->gpioValueValue;
}
void XWalkBoardControlHostStub::writeGpio(contextpointer context, uint8 pin, boolean value)
{
    static_cast<void>(pin); static_cast<XWalkBoardControlHostStub*>(context)->gpioValueValue = value;
}
void XWalkBoardControlHostStub::interruptGpio(contextpointer context, uint8 pin,
    XWalkGpioEdge edge, uint32 debounceMs, contextpointer handlerContext,
    gpiointerrupthandler handler)
{
    static_cast<void>(context); static_cast<void>(pin); static_cast<void>(edge);
    static_cast<void>(debounceMs); static_cast<void>(handlerContext); static_cast<void>(handler);
}
void XWalkBoardControlHostStub::cancelGpio(contextpointer context, uint8 pin)
{
    static_cast<void>(context); static_cast<void>(pin);
}
boolean XWalkBoardControlHostStub::probeI2c(contextpointer context, uint8 address)
{
    static_cast<void>(context); return address == XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1;
}
void XWalkBoardControlHostStub::writeI2c(contextpointer context, uint8 address,
    uint8 reg, const bytevector& data)
{
    static_cast<void>(context); static_cast<void>(address); static_cast<void>(reg);
    static_cast<void>(data);
}
bytevector XWalkBoardControlHostStub::readI2c(contextpointer context, uint8 address,
    size length)
{
    static_cast<void>(context); static_cast<void>(address);
    return length == 2U ? bytevector{0x0FU, 0xFFU} : bytevector(length, 0U);
}
bytevector XWalkBoardControlHostStub::readRegisterI2c(contextpointer context,
    uint8 address, uint8 reg, size length)
{
    static_cast<void>(context); static_cast<void>(address); static_cast<void>(reg);
    return length == 3U ? bytevector{2U, 5U, 5U} : bytevector(length, 0U);
}
void XWalkBoardControlHostStub::primeSpeaker(contextpointer context, uint32 durationMs)
{
    XWalkBoardControlHostStub& backend = *static_cast<XWalkBoardControlHostStub*>(context);
    if (durationMs == XHAL_RPI5CAR_BOARD_CONTROL_SPEAKER_PRIME_DURATION_MS)
    {
        ++backend.primeCountValue;
    }
}
XWalkGpioCallbacks XWalkBoardControlHostStub::gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelGpio};
}
boolean XWalkBoardControlHostStub::gpioValue() const noexcept { return gpioValueValue; }
uint32 XWalkBoardControlHostStub::primeCount() const noexcept { return primeCountValue; }
} /* namespace xwalk::hal::sim */
