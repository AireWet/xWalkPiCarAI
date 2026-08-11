/******************************************************************************
 * @file        xHal_Rpi5CarRobotHostStub.h
 * @brief       Declares the device-free xWalkRobot I2C adapter.
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ROBOT_HOST_STUB_H
#define XHAL_RPI5CAR_ROBOT_HOST_STUB_H
#include "xHal_Rpi5CarRobot.h"
namespace xwalk::hal::sim
{
/** @brief Records simulated servo register writes without opening an I2C device. */
class XWalkRobotHostStub final
{
    private:
        uint32 writeCountValue{};
    public:
        static boolean probe(contextpointer context, uint8 address);
        static void writeRegister(contextpointer context, uint8 address, uint8 reg,
            const bytevector& data);
        static bytevector read(contextpointer context, uint8 address, size length);
        uint32 writeCount() const noexcept;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ROBOT_HOST_STUB_H */
