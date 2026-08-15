/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerHostStub.cpp
 * @brief       Implements the device-free LineTracker I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkLineTracker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarLineTrackerHostStub.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    boolean XWalkLineTrackerHostStub::probe(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }
    void
    XWalkLineTrackerHostStub::writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(address);
        static_cast<void>(data);
        static_cast<XWalkLineTrackerHostStub*>(context)->commandValue = reg;
    }
    bytevector XWalkLineTrackerHostStub::read(contextpointer context, uint8 address, size length)
    {
        static_cast<void>(address);
        static_cast<void>(length);
        XWalkLineTrackerHostStub& self = *static_cast<XWalkLineTrackerHostStub*>(context);
        ++self.readCountValue;
        const uint8 hardwareChannel = static_cast<uint8>(self.commandValue - XHAL_RPI5CAR_ADC_READ_COMMAND);
        const uint8 logicalChannel = static_cast<uint8>(XHAL_RPI5CAR_ADC_MAX_CHANNEL - hardwareChannel);
        const uint16 sample = self.samplesValue[logicalChannel];
        XWALK_HAL_TRACE_UID0(RPI .239, "LineTracker host stub returned an ADC sample");
        return {static_cast<uint8>((sample >> 8U) & 0xFFU), static_cast<uint8>(sample & 0xFFU)};
    }
    uint32 XWalkLineTrackerHostStub::readCount() const noexcept
    {
        return readCountValue;
    }
} /* namespace xwalk::hal::sim */
