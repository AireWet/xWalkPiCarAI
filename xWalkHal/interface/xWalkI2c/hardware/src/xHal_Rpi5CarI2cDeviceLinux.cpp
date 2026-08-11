/******************************************************************************
 * @file        xHal_Rpi5CarI2cDeviceLinux.cpp
 * @brief       Implements the production Linux I2C system-call adapter.
 *
 * @details
 * Forwards device open, slave selection, SMBus transfer, and close operations
 * to the Linux kernel without adding backend policy.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Linux Backend
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

#include "xHal_Rpi5CarI2cDeviceLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal
{

XWalkI2cDeviceLinux::XWalkI2cDeviceLinux() = default;
XWalkI2cDeviceLinux::~XWalkI2cDeviceLinux() = default;

int32 XWalkI2cDeviceLinux::openDevice(cstring devicePath)
{
    return ::open(devicePath, O_RDWR | O_CLOEXEC);
}

boolean XWalkI2cDeviceLinux::selectAddress(int32 fileDescriptor, uint8 address)
{
    return ::ioctl(fileDescriptor, I2C_SLAVE, address) >= 0;
}

boolean XWalkI2cDeviceLinux::transfer(int32 fileDescriptor, contextpointer request)
{
    return ::ioctl(fileDescriptor, I2C_SMBUS, request) >= 0;
}

void XWalkI2cDeviceLinux::closeDevice(int32 fileDescriptor) noexcept
{
    static_cast<void>(::close(fileDescriptor));
}

} /* namespace xwalk::hal */
