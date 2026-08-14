/******************************************************************************
 * @file        xHal_Rpi5CarMotorTestSupport.h
 * @brief       Declares reusable xWalkMotor host-test support.
 * @project     xWalk Firmware
 * @module      xWalkMotor Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_MOTOR_TEST_SUPPORT_H
#define XHAL_RPI5CAR_MOTOR_TEST_SUPPORT_H
#include "xHal_Rpi5CarMotor.h"
namespace xwalk::hal::test::motor
{
/** @brief Deterministic monotonic clock for motor-watchdog tests. */
struct FakeClock
{
    uint64 milliseconds{};
};

/** @brief Returns the current fake monotonic time. */
uint64 clockMilliseconds(contextpointer context);
/** @brief Throws to emulate watchdog-worker startup failure before a thread is created. */
void failThreadStart(contextpointer context);
/** @brief Provides callback state required by host PWM objects. */
struct TestI2c
{
    uint32 writeCount{};
    uint32vector failingWrites;
};
/** @brief Records the most recent simulated GPIO output. */
struct TestGpio
{
    boolean value{};
    size writeCount{};
};
boolean probe(contextpointer context, uint8 address);
void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
boolean tryWriteRegister(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data) noexcept;
bytevector read(contextpointer context, uint8 address, size length);
void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode,
    XWalkGpioPull pull, boolean initialValue);
boolean readGpio(contextpointer context, uint8 pin);
void writeGpio(contextpointer context, uint8 pin, boolean value);
void interruptGpio(contextpointer context, uint8 pin, XWalkGpioEdge edge,
    uint32 debounceMs, contextpointer handlerContext, gpiointerrupthandler handler);
void cancelGpioInterrupt(contextpointer context, uint8 pin);
XWalkGpioCallbacks gpioCallbacks();
} /* namespace xwalk::hal::test::motor */
#endif /* XHAL_RPI5CAR_MOTOR_TEST_SUPPORT_H */
