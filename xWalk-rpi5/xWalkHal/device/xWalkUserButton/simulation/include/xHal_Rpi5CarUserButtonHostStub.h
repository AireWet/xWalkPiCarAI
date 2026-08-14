/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonHostStub.h
 * @brief       Declares the device-free UserButton GPIO host stub.
 * @project     xWalk Firmware
 * @module      xWalkUserButton Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_USER_BUTTON_HOST_STUB_H
#define XHAL_RPI5CAR_USER_BUTTON_HOST_STUB_H
#include "xHal_Rpi5CarUserButton.h"
namespace xwalk::hal::sim
{
/** @brief Supplies one active-low button level and records click callbacks. */
class XWalkUserButtonHostStub final
{
    private:
        atomicboolean inputLevelValue{true};
        uint32 clickCountValue{};
    public:
        static void configure(contextpointer context, uint8 pin, XWalkGpioMode mode,
            XWalkGpioPull pull, boolean initialValue);
        static boolean read(contextpointer context, uint8 pin);
        static void write(contextpointer context, uint8 pin, boolean value);
        static void interrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
            uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler);
        static void cancelInterrupt(contextpointer context, uint8 pin);
        static void countClick(contextpointer context);
        static XWalkGpioCallbacks callbacks();
        void setPressed(boolean pressed) noexcept;
        uint32 clickCount() const noexcept;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_USER_BUTTON_HOST_STUB_H */
