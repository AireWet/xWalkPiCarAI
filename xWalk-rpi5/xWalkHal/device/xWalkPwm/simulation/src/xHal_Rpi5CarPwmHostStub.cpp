/******************************************************************************
 * @file        xHal_Rpi5CarPwmHostStub.cpp
 * @brief       Implements the device-free PWM I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarPwmHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    boolean XWalkPwmHostStub::probe(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        return address == 0x14U;
    }

    void XWalkPwmHostStub::writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(address);
        XWalkPwmHostStub& self = *static_cast<XWalkPwmHostStub*>(context);
        ++self.writeCountValue;
        self.lastRegisterValue = reg;
        self.lastPayloadSizeValue = data.size();
        self.lastHighByteValue = data.empty() ? 0U : data.front();
        self.lastLowByteValue = data.size() < 2U ? 0U : data[1U];
        XWALK_HAL_TRACE_UID1(RPI .168, "PWM host stub recorded register 0x%02X", reg);
    }

    boolean XWalkPwmHostStub::tryWriteRegister(contextpointer context,
                                               uint8 address,
                                               uint8 reg,
                                               const bytevector& data) noexcept
    {
        static_cast<void>(address);
        XWalkPwmHostStub& self = *static_cast<XWalkPwmHostStub*>(context);
        ++self.writeCountValue;
        self.lastRegisterValue = reg;
        self.lastPayloadSizeValue = data.size();
        self.lastHighByteValue = data.empty() ? 0U : data.front();
        self.lastLowByteValue = data.size() < 2U ? 0U : data[1U];
        return true;
    }

    bytevector XWalkPwmHostStub::read(contextpointer context, uint8 address, size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return bytevector(length, 0U);
    }

    size XWalkPwmHostStub::writeCount() const noexcept
    {
        return writeCountValue;
    }
    uint8 XWalkPwmHostStub::lastRegister() const noexcept
    {
        return lastRegisterValue;
    }
    size XWalkPwmHostStub::lastPayloadSize() const noexcept
    {
        return lastPayloadSizeValue;
    }
    uint8 XWalkPwmHostStub::lastHighByte() const noexcept
    {
        return lastHighByteValue;
    }
    uint8 XWalkPwmHostStub::lastLowByte() const noexcept
    {
        return lastLowByteValue;
    }
} /* namespace xwalk::hal::sim */
