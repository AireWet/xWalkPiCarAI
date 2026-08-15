/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonTestSupport.h
 * @brief       Declares reusable user-button host-test support.
 * @project     xWalk Firmware
 * @module      xWalkUserButton Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_USER_BUTTON_TEST_SUPPORT_H
#define XHAL_RPI5CAR_USER_BUTTON_TEST_SUPPORT_H
#include "xHal_Rpi5CarUserButton.h"
namespace xwalk::hal::test::userbutton
{
    /** @brief Supplies an atomic simulated input level and optional read failure. */
    struct TestBackend
    {
            atomicboolean inputLevel{true};
            atomicboolean failReads{false};
    };

    /** @brief Counts each user-button callback observation. */
    struct EventCounts
    {
            uint32 clicks{};
            uint32 presses{};
            uint32 releases{};
            uint32 pressedStates{};
            uint32 releasedStates{};
            uint32 longPresses{};
            uint32 longReleases{};
    };

    void configure(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue);
    boolean read(contextpointer context, uint8 pin);
    void write(contextpointer context, uint8 pin, boolean value);
    void registerInterrupt(contextpointer context,
                           uint8 pin,
                           XWalkGpioEdge edge,
                           uint32 debounceMs,
                           contextpointer handlerContext,
                           gpiointerrupthandler handler);
    void cancelInterrupt(contextpointer context, uint8 pin);
    XWalkGpioCallbacks gpioCallbacks();
    void countClick(contextpointer context);
    void countPress(contextpointer context);
    void countRelease(contextpointer context);
    void countLongPress(contextpointer context);
    void countLongRelease(contextpointer context);
    void countState(contextpointer context, boolean pressed);
} /* namespace xwalk::hal::test::userbutton */
#endif /* XHAL_RPI5CAR_USER_BUTTON_TEST_SUPPORT_H */
