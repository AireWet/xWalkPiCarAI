/******************************************************************************
 * @file        xHal_Rpi5CarRobotTestSupport.cpp
 * @brief       Implements reusable xWalkRobot host-test support.
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarRobotTestSupport.h"
namespace xwalk::hal::test::robot
{
boolean probe(contextpointer context, uint8 address)
{
    static_cast<void>(context); static_cast<void>(address); return true;
}
void writeRegister(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data)
{
    static_cast<void>(address); static_cast<void>(reg); static_cast<void>(data);
    ++static_cast<TestBus*>(context)->writeCount;
}
bytevector read(contextpointer context, uint8 address, size length)
{
    static_cast<void>(context); static_cast<void>(address); return bytevector(length, 0U);
}
} /* namespace xwalk::hal::test::robot */
