/******************************************************************************
 * @file        xHal_Rpi5CarRobotHostStub.cpp
 * @brief       Implements the device-free xWalkRobot I2C adapter.
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarRobotHostStub.h"
namespace xwalk::hal::sim
{
    boolean XWalkRobotHostStub::probe(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }
    void XWalkRobotHostStub::writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
        ++static_cast<XWalkRobotHostStub*>(context)->writeCountValue;
    }
    bytevector XWalkRobotHostStub::read(contextpointer context, uint8 address, size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return bytevector(length, 0U);
    }
    uint32 XWalkRobotHostStub::writeCount() const noexcept
    {
        return writeCountValue;
    }
} /* namespace xwalk::hal::sim */
