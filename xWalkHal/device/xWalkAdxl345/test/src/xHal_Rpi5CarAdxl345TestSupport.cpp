/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345TestSupport.cpp
 * @brief       Implements reusable ADXL345 host-test I2C support.
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarAdxl345TestSupport.h"
#include "xHal_Rpi5CarAdxl345.h"
namespace xwalk::hal::test::adxl345
{
boolean probe(contextpointer context, uint8 address)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.lastAddress = address;
    return address == XHAL_RPI5CAR_ADXL345_ADDRESS;
}

void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.lastAddress = address;
    bus.lastRegister = reg;
    if ((reg == XHAL_RPI5CAR_ADXL345_DATA_FORMAT_REGISTER) &&
        (data == bytevector({XHAL_RPI5CAR_ADXL345_DATA_FORMAT_VALUE})))
    {
        ++bus.formatWriteCount;
    }
    if ((reg == XHAL_RPI5CAR_ADXL345_POWER_CONTROL_REGISTER) &&
        (data == bytevector({XHAL_RPI5CAR_ADXL345_MEASUREMENT_MODE_VALUE})))
    {
        ++bus.powerWriteCount;
    }
}

bytevector read(contextpointer context, uint8 address, size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(length);
    return {0U};
}

bytevector readRegister(contextpointer context, uint8 address, uint8 reg, size length)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.lastAddress = address;
    bus.lastRegister = reg;
    bus.lastLength = length;
    ++bus.registerReadCount;
    if (bus.responseIndex >= bus.responses.size())
    {
        return {};
    }
    const bytevector response = bus.responses[bus.responseIndex];
    ++bus.responseIndex;
    return response;
}
} /* namespace xwalk::hal::test::adxl345 */
