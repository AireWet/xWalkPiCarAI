/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345Simulation.cpp
 * @brief       Implements the device-free xWalkAdxl345 simulation.
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarAdxl345Simulation.h"
#include "xHal_Rpi5CarAdxl345.h"
#include "xHal_Rpi5CarAdxl345HostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    int32 runAdxl345Simulation()
    {
        XWalkAdxl345HostStub backend;
        XWalkI2c i2c(&backend,
                     &XWalkAdxl345HostStub::probe,
                     &XWalkAdxl345HostStub::writeRegister,
                     &XWalkAdxl345HostStub::read,
                     &XWalkAdxl345HostStub::readRegister);
        XWalkAdxl345 accelerometer(i2c);
        const adxl345values values = accelerometer.read();
        const boolean valid = (values == adxl345values({1.0, 0.0, -1.0})) && (backend.writeCount() == 6U) &&
                              (backend.registerReadCount() == 6U);
        XWALK_HAL_TRACE_UID0(RPI .197, "xWalkAdxl345 host simulation completed");
        return valid ? 0 : 1;
    }
} /* namespace xwalk::hal::sim */
