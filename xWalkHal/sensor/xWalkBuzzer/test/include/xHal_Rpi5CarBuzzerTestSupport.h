/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerTestSupport.h
 * @brief       Declares reusable xWalkBuzzer host-test support.
 * @project     xWalk Firmware
 * @module      xWalkBuzzer Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_BUZZER_TEST_SUPPORT_H
#define XHAL_RPI5CAR_BUZZER_TEST_SUPPORT_H
#include "xHal_Rpi5CarBuzzer.h"
namespace xwalk::hal::test::buzzer
{
/** @brief Records simulated I2C and GPIO output traffic. */
struct TestBackend
{
    uint32 i2cWriteCount{};
    uint32 gpioConfigureCount{};
    uint32 gpioWriteCount{};
    boolean gpioValue{};
};
/** @brief Owns the caller-created dependency graph for a passive buzzer. */
struct PassiveFixture
{
    TestBackend backend;
    XWalkI2c i2c;
    XWalkPwmTimerState timerState;
    XWalkPwm pwm;
    PassiveFixture();
};
boolean probe(contextpointer context, uint8 address);
void writeRegister(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data);
bytevector read(contextpointer context, uint8 address, size length);
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue);
boolean readGpio(contextpointer context, uint8 pin);
void writeGpio(contextpointer context, uint8 pin, boolean value);
void registerInterrupt(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler);
void cancelInterrupt(contextpointer context, uint8 pin);
XWalkGpioCallbacks gpioCallbacks();
} /* namespace xwalk::hal::test::buzzer */
#endif /* XHAL_RPI5CAR_BUZZER_TEST_SUPPORT_H */
