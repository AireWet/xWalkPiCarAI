/******************************************************************************
 * @file        xHal_Rpi5CarRobotTestSupport.h
 * @brief       Declares reusable xWalkRobot host-test support.
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ROBOT_TEST_SUPPORT_H
#define XHAL_RPI5CAR_ROBOT_TEST_SUPPORT_H
#include "xHal_Rpi5CarRobot.h"
namespace xwalk::hal::test::robot
{
/** @brief Records simulated Robot HAT register writes. */
struct TestBus
{
    uint32 writeCount{};
};
boolean probe(contextpointer context, uint8 address);
void writeRegister(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data);
bytevector read(contextpointer context, uint8 address, size length);
} /* namespace xwalk::hal::test::robot */
#endif /* XHAL_RPI5CAR_ROBOT_TEST_SUPPORT_H */
