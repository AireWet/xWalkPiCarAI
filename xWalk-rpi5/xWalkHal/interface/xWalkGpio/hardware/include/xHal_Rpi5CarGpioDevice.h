/******************************************************************************
 * @file        xHal_Rpi5CarGpioDevice.h
 * @brief       Declares the injectable Linux GPIO device-operation boundary.
 *
 * @details
 * Separates Linux descriptor, ioctl, polling, and event-read operations from
 * the GPIO ownership and debounce behavior implemented by `XWalkGpioLinux`.
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

#ifndef XHAL_RPI5CAR_GPIO_DEVICE_H
#define XHAL_RPI5CAR_GPIO_DEVICE_H

#include "xHal_Rpi5CarCommon.h"

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /** @brief Abstracts operating-system calls used by the Linux GPIO backend. */
    class XWalkGpioDevice
    {
        public:
            /** @brief Allows destruction through the interface. */
            virtual ~XWalkGpioDevice() = default;

            /** @brief Opens and returns an owned GPIO chip descriptor. */
            virtual int32 openDevice(cstring devicePath) = 0;

            /** @brief Reads Linux GPIO chip information into an opaque request structure. */
            virtual boolean readChipInformation(int32 chipDescriptor, contextpointer information) = 0;

            /** @brief Claims one GPIO line through an opaque handle request. */
            virtual boolean requestLine(int32 chipDescriptor, contextpointer request) = 0;

            /** @brief Samples one claimed GPIO line into an opaque data structure. */
            virtual boolean readLine(int32 lineDescriptor, contextpointer data) = 0;

            /** @brief Drives one claimed GPIO line from an opaque data structure. */
            virtual boolean writeLine(int32 lineDescriptor, contextpointer data) = 0;

            /** @brief Claims one GPIO event line through an opaque event request. */
            virtual boolean requestEvent(int32 chipDescriptor, contextpointer request) = 0;

            /** @brief Polls one event descriptor for the supplied timeout in milliseconds. */
            virtual int32 pollEvent(int32 lineDescriptor, int32 timeoutMs) = 0;

            /** @brief Reads one event record into an opaque buffer. */
            virtual int32 readEvent(int32 lineDescriptor, contextpointer eventData, size length) = 0;

            /** @brief Closes an owned descriptor without throwing. */
            virtual void closeDevice(int32 fileDescriptor) noexcept = 0;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_GPIO_DEVICE_H */
