/******************************************************************************
 * @file        xHal_Rpi5CarGpioLinuxLifecycle.cpp
 * @brief       Implements Linux GPIO backend lifecycle behavior.
 *
 * @details
 * Opens the GPIO chip device, stops the optional event worker, and closes owned descriptors.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Linux Backend
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Opens a Linux GPIO chip device.
 *
 * @param[in] devicePath
 * Non-null path to the GPIO character device.
 *
 * @param[in] expectedName
 * Optional exact kernel chip name verified when non-empty.
 *
 * @param[in] expectedLabel
 * Optional exact kernel chip label verified when non-empty.
 *
 * @param[in] minimumLineCount
 * Minimum number of lines required from the selected controller.
 *
 * @return
 * Owned non-negative Linux file descriptor.
 *
 * @throws std::invalid_argument
 * If `devicePath` is null.
 *
 * @throws std::runtime_error
 * If the device cannot be opened.
 */
int32 XWalkGpioLinux::openDevice(cstring devicePath, stringview expectedName,
    stringview expectedLabel, uint32 minimumLineCount)
{
    if ((devicePath == nullptr) || (devicePath[0U] == '\0'))
    {
        XHAL_THROW_INVALID_ARGUMENT("GPIO device path must not be empty");
    }
    const int32 descriptor = ::open(devicePath, O_RDWR | O_CLOEXEC);
    if (descriptor < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("Unable to open Linux GPIO device");
    }
    gpiochip_info information{};
    const hal::boolean chipInformationReadFailed =
        static_cast<hal::boolean>(
            ::ioctl(descriptor, GPIO_GET_CHIPINFO_IOCTL, &information) < 0);
    if (chipInformationReadFailed)
    {
        static_cast<void>(::close(descriptor));
        XHAL_THROW_RUNTIME_ERROR("Unable to inspect Linux GPIO device");
    }
    const string chipName{information.name};
    const string chipLabel{information.label};
    const hal::boolean expectedNameChipNameExpectedLabelInvalid =
        static_cast<hal::boolean>(
            (!expectedName.empty() && (chipName != expectedName)) ||
        (!expectedLabel.empty() && (chipLabel != expectedLabel)) ||
        (information.lines < minimumLineCount));
    if (expectedNameChipNameExpectedLabelInvalid)
    {
        static_cast<void>(::close(descriptor));
        XHAL_THROW_RUNTIME_ERROR("Linux GPIO device identity or line count does not match configuration");
    }
    return descriptor;
}

/**
 * @brief Closes the owned line descriptor when one is open.
 *
 * @post
 * `lineDescriptor` is `-1`.
 */
void XWalkGpioLinux::releaseLine() noexcept
{
    if (lineDescriptor >= 0)
    {
        static_cast<void>(::close(lineDescriptor));
        lineDescriptor = -1;
    }
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a Linux GPIO backend and opens its chip device.
 *
 * @param[in] devicePath
 * Non-null path to the GPIO character device.
 *
 * @param[in] expectedName
 * Optional exact kernel chip name verified before line claims.
 *
 * @param[in] expectedLabel
 * Optional exact kernel chip label verified before line claims.
 *
 * @param[in] minimumLineCount
 * Minimum controller line count required by the application graph.
 *
 * @post
 * The backend owns an open chip descriptor but no GPIO line.
 */
XWalkGpioLinux::XWalkGpioLinux(cstring devicePath, stringview expectedName,
    stringview expectedLabel, uint32 minimumLineCount):
    chipDescriptor(openDevice(devicePath, expectedName, expectedLabel, minimumLineCount))
{
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Stops event dispatch and closes all owned Linux descriptors.
 *
 * @post
 * The event worker is joined and both descriptors are closed.
 */
XWalkGpioLinux::~XWalkGpioLinux()
{
    stopRequested.store(true);
    const hal::boolean eventThreadJoinable =
        static_cast<hal::boolean>(
            eventThread.joinable());
    if (eventThreadJoinable)
    {
        eventThread.join();
    }
    releaseLine();
    if (chipDescriptor >= 0)
    {
        static_cast<void>(::close(chipDescriptor));
        chipDescriptor = -1;
    }
}

} /* namespace xwalk::hal */
