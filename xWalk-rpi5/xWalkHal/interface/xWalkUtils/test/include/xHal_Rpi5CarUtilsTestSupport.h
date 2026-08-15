/******************************************************************************
 * @file        xHal_Rpi5CarUtilsTestSupport.h
 * @brief       Declares reusable xWalkUtils host-test support.
 * @details     Defines injected utility, lazy-reader, and redirect state and callbacks.
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_UTILS_TEST_SUPPORT_H
#define XHAL_RPI5CAR_UTILS_TEST_SUPPORT_H
#include "xHal_Rpi5CarUtils.h"
namespace xwalk::hal::test::utils
{
    struct TestUtilsBackend
    {
            XWalkUtilityColor color{XWalkUtilityColor::White};
            string message{};
            string ending{};
            string command{};
            string user{};
            string group{};
            string executable{};
            string interfaceName{};
            uint8 volumePercent{};
            boolean flush{};
            uint32 outputCount{};
    };
    struct TestLazyBackend
    {
            uint64 currentTimeUs{};
            uint32 nextValue{10U};
            uint32 readCount{};
    };
    struct TestRedirectBackend
    {
            uint32 redirectCount{};
            uint32 restoreCount{};
            int32 token{};
    };
    void
    writeOutput(contextpointer context, XWalkUtilityColor color, stringview message, stringview ending, boolean flush);
    void setVolume(contextpointer context, uint8 volumePercent);
    XWalkCommandResult runCommand(contextpointer context, stringview command, stringview user, stringview group);
    boolean executableExists(contextpointer context, stringview executable);
    string ipAddress(contextpointer context, stringview interfaceName);
    string username(contextpointer context);
    XWalkUtilsCallbacks utilityCallbacks();
    uint64 lazyClock(contextpointer context);
    uint32 lazyRead(contextpointer context);
    int32 redirectError(contextpointer context);
    void restoreError(contextpointer context, int32 restoreToken);
} /* namespace xwalk::hal::test::utils */
#endif /* XHAL_RPI5CAR_UTILS_TEST_SUPPORT_H */
