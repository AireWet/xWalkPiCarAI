/******************************************************************************
 * @file        xHal_Rpi5CarAdcTestSupport.cpp
 * @brief       Implements reusable ADC host-test I2C support.
 * @project     xWalk Firmware
 * @module      xWalkAdc Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarAdcTestSupport.h"
namespace xwalk::hal::test::adc
{
    boolean probe(contextpointer context, uint8 address)
    {
        TestBus& bus = *static_cast<TestBus*>(context);
        bus.probes.push_back(address);
        return bus.presentAddresses.count(address) != 0U;
    }

    void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        TestBus& bus = *static_cast<TestBus*>(context);
        uniquemutexlock lock(bus.mutexValue);
        bus.writeAddress = address;
        bus.writeRegister = reg;
        bus.writeData = data;
        bus.selectedCommand = reg;
        bus.operationOrder.push_back(reg);
        if (bus.pauseFirstWrite && !bus.firstWriteObserved)
        {
            bus.firstWriteObserved = true;
            bus.conditionValue.notify_all();
            bus.conditionValue.wait(lock,
                                    [&bus]()
                                    {
                                        return bus.releaseFirstWrite;
                                    });
        }
    }

    bytevector read(contextpointer context, uint8 address, size length)
    {
        TestBus& bus = *static_cast<TestBus*>(context);
        const mutexlock lock(bus.mutexValue);
        bus.readAddress = address;
        bus.readLength = length;
        bus.operationOrder.push_back(0U);
        if (bus.returnSelectedChannelValue)
        {
            const uint8 channel = static_cast<uint8>(7U - (bus.selectedCommand & 0x07U));
            const uint16 value = bus.channelValues[channel];
            return {static_cast<uint8>((value >> 8U) & 0xFFU), static_cast<uint8>(value & 0xFFU)};
        }
        return bus.readBytes;
    }
} /* namespace xwalk::hal::test::adc */
