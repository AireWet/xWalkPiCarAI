/******************************************************************************
 * @file        xHal_Rpi5CarI2cLinuxLifecycle.cpp
 * @brief       Implements Linux I2C backend validation and lifecycle behavior.
 *
 * @details
 * Validates retry configuration, opens the Linux I2C device, and closes the
 * owned descriptor during destruction. Callback composition remains outside
 * the backend.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Linux Backend
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

#include "xHal_Rpi5CarI2cLinux.h"

#include "xHal_Rpi5CarCommon.h"
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
 * @brief Validates the configured I2C operation attempt count.
 *
 * @param[in] retryCount
 * Number of permitted attempts.
 *
 * @return
 * The validated non-zero attempt count.
 *
 * @throws std::invalid_argument
 * If `retryCount` is zero.
 */
uint32 XWalkI2cLinux::validateRetryCount(uint32 retryCount)
{
    if (retryCount == 0U)
    {
        XHAL_THROW_INVALID_ARGUMENT("I2C retry count must be greater than zero");
    }
    return retryCount;
}

/**
 * @brief Opens a Linux I2C device node for owned read/write access.
 *
 * @param[in] devicePath
 * Non-null path to the I2C device node.
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
int32 XWalkI2cLinux::openDevice(cstring devicePath)
{
    if (devicePath == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("I2C device path must not be null");
    }

    const int32 descriptor = ::open(devicePath, O_RDWR | O_CLOEXEC);
    if (descriptor < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("Unable to open Linux I2C device");
    }
    return descriptor;
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a Linux I2C backend and opens its device node.
 *
 * @param[in] devicePath
 * Non-null path to the Linux I2C device node.
 *
 * @param[in] retryCount
 * Number of probe, write, or read attempts; valid range is 1 to `UINT32_MAX`.
 *
 * @post
 * On successful construction, the backend owns an open descriptor and is ready
 * to receive calls from an externally created callback interface.
 *
 * @throws std::invalid_argument
 * If `devicePath` is null or `retryCount` is zero.
 *
 * @throws std::runtime_error
 * If the Linux device node cannot be opened.
 */
XWalkI2cLinux::XWalkI2cLinux(cstring devicePath, uint32 retryCount):
    retryCountValue(validateRetryCount(retryCount)), fileDescriptor(openDevice(devicePath))
{
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the backend and closes its owned descriptor.
 *
 * @post
 * A descriptor that was non-negative is passed to `close` and stored as `-1`.
 *
 * @note
 * The return value from the Linux `close` operation is not inspected.
 */
XWalkI2cLinux::~XWalkI2cLinux()
{
    if (fileDescriptor >= 0)
    {
        ::close(fileDescriptor);
        fileDescriptor = -1;
    }
}

} /* namespace xwalk::hal */
