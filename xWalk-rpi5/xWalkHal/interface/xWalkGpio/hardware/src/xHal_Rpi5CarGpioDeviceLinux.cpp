/******************************************************************************
 * @file        xHal_Rpi5CarGpioDeviceLinux.cpp
 * @brief       Implements the production Linux GPIO system-call adapter.
 *
 * @details
 * Forwards GPIO device, ioctl, polling, event-read, and close operations to
 * the Linux kernel without adding backend policy.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Linux Backend
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarGpioDeviceLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    XWalkGpioDeviceLinux::XWalkGpioDeviceLinux() = default;
    XWalkGpioDeviceLinux::~XWalkGpioDeviceLinux() = default;

    int32 XWalkGpioDeviceLinux::openDevice(cstring devicePath)
    {
        return ::open(devicePath, O_RDWR | O_CLOEXEC);
    }

    boolean XWalkGpioDeviceLinux::readChipInformation(int32 chipDescriptor, contextpointer information)
    {
        return ::ioctl(chipDescriptor, GPIO_GET_CHIPINFO_IOCTL, information) >= 0;
    }

    boolean XWalkGpioDeviceLinux::requestLine(int32 chipDescriptor, contextpointer request)
    {
        return ::ioctl(chipDescriptor, GPIO_GET_LINEHANDLE_IOCTL, request) >= 0;
    }

    boolean XWalkGpioDeviceLinux::readLine(int32 lineDescriptor, contextpointer data)
    {
        return ::ioctl(lineDescriptor, GPIOHANDLE_GET_LINE_VALUES_IOCTL, data) >= 0;
    }

    boolean XWalkGpioDeviceLinux::writeLine(int32 lineDescriptor, contextpointer data)
    {
        return ::ioctl(lineDescriptor, GPIOHANDLE_SET_LINE_VALUES_IOCTL, data) >= 0;
    }

    boolean XWalkGpioDeviceLinux::requestEvent(int32 chipDescriptor, contextpointer request)
    {
        return ::ioctl(chipDescriptor, GPIO_GET_LINEEVENT_IOCTL, request) >= 0;
    }

    int32 XWalkGpioDeviceLinux::pollEvent(int32 lineDescriptor, int32 timeoutMs)
    {
        pollfd descriptorEvent{};
        descriptorEvent.fd = lineDescriptor;
        descriptorEvent.events = POLLIN;
        const int32 pollResult = static_cast<int32>(::poll(&descriptorEvent, 1U, timeoutMs));
        if (pollResult <= 0)
        {
            return pollResult;
        }
        return (descriptorEvent.revents & POLLIN) != 0 ? 1 : 0;
    }

    int32 XWalkGpioDeviceLinux::readEvent(int32 lineDescriptor, contextpointer eventData, size length)
    {
        return static_cast<int32>(::read(lineDescriptor, eventData, length));
    }

    void XWalkGpioDeviceLinux::closeDevice(int32 fileDescriptor) noexcept
    {
        static_cast<void>(::close(fileDescriptor));
    }

} /* namespace xwalk::hal */
