/******************************************************************************
 * @file        xHal_Rpi5CarI2cHandler.cpp
 * @brief       Implements the standalone I2C operation simulation handler.
 *
 * @details
 * Exercises the public I2C operation sequence without test assertions or test
 * framework dependencies.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-09
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2cHandler.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim
{

XWalkI2cHandler::XWalkI2cHandler() = default;
XWalkI2cHandler::~XWalkI2cHandler() = default;

int32 XWalkI2cHandler::run(XWalkI2c& i2c) const
{
    constexpr uint8 deviceAddress = 0x14U;
    constexpr uint8 registerAddress = 0x20U;
    constexpr size readLength = 2U;
    const bytevector writeBytes{0x12U, 0x34U};

    const boolean deviceAvailable = i2c.probe(deviceAddress);
    if (deviceAvailable == false)
    {
        XWALK_HAL_WARNINGS("I2C simulation device did not respond at address %u",
            static_cast<uint32>(deviceAddress));
        return 1;
    }

    i2c.writeRegister(deviceAddress, registerAddress, writeBytes);
    const boolean safeWriteSucceeded =
        i2c.tryWriteRegister(deviceAddress, registerAddress, writeBytes);
    if (safeWriteSucceeded == false)
    {
        XWALK_HAL_WARNINGS("I2C simulation fail-safe write was rejected");
        return 2;
    }

    const bytevector sequentialBytes = i2c.read(deviceAddress, readLength);
    const bytevector registerBytes =
        i2c.readRegister(deviceAddress, registerAddress, readLength);
    XWALK_HAL_TRACE_UID1(RPI.044,
        "xWalkI2c simulation completed: sequential=%zu register=%zu",
        sequentialBytes.size(), registerBytes.size());
    return 0;
}

} /* namespace xwalk::hal::sim */
