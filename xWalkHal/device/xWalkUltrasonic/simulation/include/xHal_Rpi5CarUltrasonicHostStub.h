/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicHostStub.h
 * @brief       Declares the device-free ultrasonic GPIO host stub.
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ULTRASONIC_HOST_STUB_H
#define XHAL_RPI5CAR_ULTRASONIC_HOST_STUB_H
#include "xHal_Rpi5CarGpio.h"
namespace xwalk::hal::sim
{
/** @brief Produces one deterministic echo pulse entirely in memory. */
class XWalkUltrasonicHostStub final
{
    private:
        uint32 echoReadCountValue{};
        uint32 triggerCountValue{};
        uint32 triggerWriteCountValue{};
    public:
        static void configure(contextpointer context, uint8 pin, XWalkGpioMode mode,
            XWalkGpioPull pull, boolean initialValue);
        static boolean read(contextpointer context, uint8 pin);
        static void write(contextpointer context, uint8 pin, boolean value);
        static void interrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
            uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler);
        static void cancelInterrupt(contextpointer context, uint8 pin);
        static XWalkGpioCallbacks callbacks();
        uint32 triggerCount() const noexcept;
        uint32 triggerWriteCount() const noexcept;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ULTRASONIC_HOST_STUB_H */
