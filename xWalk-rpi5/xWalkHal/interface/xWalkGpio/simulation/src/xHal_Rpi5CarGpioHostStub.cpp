/******************************************************************************
 * @file        xHal_Rpi5CarGpioHostStub.cpp
 * @brief       Implements the Linux GPIO device-interface host mirror.
 *
 * @details
 * Supplies deterministic chip information and mirrors line configuration,
 * input, output, and event requests into owned host state.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Simulation
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

#include "xHal_Rpi5CarGpioHostStub.h"

#include "xHal_Rpi5CarLinuxHeaders.h"
#include "xHal_Rpi5CarTrace.h"

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkGpio simulation
 * support.
 */
namespace xwalk::hal::sim
{

    XWalkGpioHostStub::XWalkGpioHostStub() = default;
    XWalkGpioHostStub::~XWalkGpioHostStub() = default;

    int32 XWalkGpioHostStub::openDevice(cstring devicePath)
    {
        return devicePath == nullptr ? -1 : 201;
    }

    boolean XWalkGpioHostStub::readChipInformation(int32 chipDescriptor, contextpointer information)
    {
        const boolean requestValid = (chipDescriptor == 201) && (information != nullptr);
        if (requestValid == false)
        {
            return false;
        }
        auto* const chipInformation = static_cast<gpiochip_info*>(information);
        const char chipName[] = "host-gpio";
        const char chipLabel[] = "xwalk-stub";
        for (size index = 0U; index < sizeof(chipName); ++index)
        {
            chipInformation->name[index] = chipName[index];
        }
        for (size index = 0U; index < sizeof(chipLabel); ++index)
        {
            chipInformation->label[index] = chipLabel[index];
        }
        chipInformation->lines = 64U;
        return true;
    }

    boolean XWalkGpioHostStub::requestLine(int32 chipDescriptor, contextpointer request)
    {
        const boolean requestValid = (chipDescriptor == 201) && (request != nullptr);
        if (requestValid == false)
        {
            return false;
        }
        auto* const lineRequest = static_cast<gpiohandle_request*>(request);
        lineRequest->fd = 202;
        pinValue = static_cast<uint8>(lineRequest->lineoffsets[0U]);
        const boolean inputRequested = (lineRequest->flags & static_cast<uint32>(GPIOHANDLE_REQUEST_INPUT)) != 0U;
        modeValue = inputRequested ? XWalkGpioMode::Input : XWalkGpioMode::Output;
        const boolean pullUpRequested =
            (lineRequest->flags & static_cast<uint32>(GPIOHANDLE_REQUEST_BIAS_PULL_UP)) != 0U;
        const boolean pullDownRequested =
            (lineRequest->flags & static_cast<uint32>(GPIOHANDLE_REQUEST_BIAS_PULL_DOWN)) != 0U;
        pullValue =
            pullUpRequested ? XWalkGpioPull::Up : (pullDownRequested ? XWalkGpioPull::Down : XWalkGpioPull::None);
        if (inputRequested == false)
        {
            lineValue = lineRequest->default_values[0U] != 0U;
        }
        ++configureCountValue;
        XWALK_HAL_TRACE_UID1(RPI .077, "Host GPIO mirrored line %u configuration", static_cast<uint32>(pinValue));
        return true;
    }

    boolean XWalkGpioHostStub::readLine(int32 lineDescriptor, contextpointer data)
    {
        const boolean requestValid = (lineDescriptor == 202) && (data != nullptr);
        if (requestValid == false)
        {
            return false;
        }
        auto* const lineData = static_cast<gpiohandle_data*>(data);
        lineData->values[0U] = lineValue ? 1U : 0U;
        ++readCountValue;
        return true;
    }

    boolean XWalkGpioHostStub::writeLine(int32 lineDescriptor, contextpointer data)
    {
        const boolean requestValid = (lineDescriptor == 202) && (data != nullptr);
        if (requestValid == false)
        {
            return false;
        }
        const auto* const lineData = static_cast<gpiohandle_data*>(data);
        lineValue = lineData->values[0U] != 0U;
        ++writeCountValue;
        return true;
    }

    boolean XWalkGpioHostStub::requestEvent(int32 chipDescriptor, contextpointer request)
    {
        const boolean requestValid = (chipDescriptor == 201) && (request != nullptr);
        if (requestValid == false)
        {
            return false;
        }
        auto* const eventRequest = static_cast<gpioevent_request*>(request);
        eventRequest->fd = 203;
        pinValue = static_cast<uint8>(eventRequest->lineoffset);
        return true;
    }

    int32 XWalkGpioHostStub::pollEvent(int32 lineDescriptor, int32 timeoutMs)
    {
        static_cast<void>(lineDescriptor);
        static_cast<void>(timeoutMs);
        return 0;
    }

    int32 XWalkGpioHostStub::readEvent(int32 lineDescriptor, contextpointer eventData, size length)
    {
        static_cast<void>(lineDescriptor);
        static_cast<void>(eventData);
        static_cast<void>(length);
        return 0;
    }

    void XWalkGpioHostStub::closeDevice(int32 fileDescriptor) noexcept
    {
        static_cast<void>(fileDescriptor);
    }

    void XWalkGpioHostStub::setInputValue(boolean value) noexcept
    {
        lineValue = value;
    }

    uint8 XWalkGpioHostStub::pin() const noexcept
    {
        return pinValue;
    }
    XWalkGpioMode XWalkGpioHostStub::mode() const noexcept
    {
        return modeValue;
    }
    XWalkGpioPull XWalkGpioHostStub::pull() const noexcept
    {
        return pullValue;
    }
    boolean XWalkGpioHostStub::value() const noexcept
    {
        return lineValue;
    }
    size XWalkGpioHostStub::configureCount() const noexcept
    {
        return configureCountValue;
    }
    size XWalkGpioHostStub::readCount() const noexcept
    {
        return readCountValue;
    }
    size XWalkGpioHostStub::writeCount() const noexcept
    {
        return writeCountValue;
    }

} /* namespace xwalk::hal::sim */
