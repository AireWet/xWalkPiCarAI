/******************************************************************************
 * @file        xHal_Rpi5CarUtilsTestSupport.cpp
 * @brief       Implements reusable xWalkUtils host-test support.
 * @details     Supplies deterministic callbacks without platform side effects.
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarUtilsTestSupport.h"
namespace xwalk::hal::test::utils
{
void writeOutput(contextpointer context, XWalkUtilityColor color,
    stringview message, stringview ending, boolean flush)
{
    auto& backend = *static_cast<TestUtilsBackend*>(context);
    backend.color = color;
    backend.message = string(message);
    backend.ending = string(ending);
    backend.flush = flush;
    ++backend.outputCount;
}
void setVolume(contextpointer context, uint8 volumePercent)
{
    static_cast<TestUtilsBackend*>(context)->volumePercent = volumePercent;
}
XWalkCommandResult runCommand(contextpointer context, stringview command,
    stringview user, stringview group)
{
    auto& backend = *static_cast<TestUtilsBackend*>(context);
    backend.command = string(command);
    backend.user = string(user);
    backend.group = string(group);
    return {7, "combined output"};
}
boolean executableExists(contextpointer context, stringview executable)
{
    auto& backend = *static_cast<TestUtilsBackend*>(context);
    backend.executable = string(executable);
    return executable == "available";
}
string ipAddress(contextpointer context, stringview interfaceName)
{
    auto& backend = *static_cast<TestUtilsBackend*>(context);
    backend.interfaceName = string(interfaceName);
    return (interfaceName == "eth0") ? string("192.0.2.10") : string{};
}
string username(contextpointer context)
{
    static_cast<void>(context);
    return "robot";
}
XWalkUtilsCallbacks utilityCallbacks()
{
    return {&writeOutput, &setVolume, &runCommand, &executableExists, &ipAddress, &username};
}
uint64 lazyClock(contextpointer context)
{
    return static_cast<TestLazyBackend*>(context)->currentTimeUs;
}
uint32 lazyRead(contextpointer context)
{
    auto& backend = *static_cast<TestLazyBackend*>(context);
    ++backend.readCount;
    const uint32 value = backend.nextValue;
    ++backend.nextValue;
    return value;
}
int32 redirectError(contextpointer context)
{
    ++static_cast<TestRedirectBackend*>(context)->redirectCount;
    return 42;
}
void restoreError(contextpointer context, int32 restoreToken)
{
    auto& backend = *static_cast<TestRedirectBackend*>(context);
    ++backend.restoreCount;
    backend.token = restoreToken;
}
} /* namespace xwalk::hal::test::utils */
