/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerTestSupport.cpp
 * @brief       Implements reusable line-tracker host-test support.
 * @project     xWalk Firmware
 * @module      xWalkLineTracker Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarLineTrackerTestSupport.h"
namespace xwalk::hal::test::linetracker
{
boolean probe(contextpointer context, uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}
void writeRegister(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data)
{
    static_cast<void>(address);
    static_cast<void>(data);
    static_cast<TestBus*>(context)->command = reg;
}
bytevector read(contextpointer context, uint8 address, size length)
{
    static_cast<void>(address);
    static_cast<void>(length);
    TestBus& bus = *static_cast<TestBus*>(context);
    ++bus.readCount;
    const uint8 hardwareChannel = static_cast<uint8>(bus.command - XHAL_RPI5CAR_ADC_READ_COMMAND);
    const uint8 logicalChannel = static_cast<uint8>(XHAL_RPI5CAR_ADC_MAX_CHANNEL - hardwareChannel);
    const uint16 sample = bus.samples[logicalChannel];
    const uint16 highValue = static_cast<uint16>(sample >> 8U);
    return {static_cast<uint8>(highValue & 0xFFU), static_cast<uint8>(sample & 0xFFU)};
}
} /* namespace xwalk::hal::test::linetracker */
