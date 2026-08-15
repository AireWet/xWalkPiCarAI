/******************************************************************************
 * @file        xHal_Rpi5CarLedTestSupport.h
 * @brief       Declares reusable xWalkLed host-test support.
 * @project     xWalk Firmware
 * @module      xWalkLed Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LED_TEST_SUPPORT_H
#define XHAL_RPI5CAR_LED_TEST_SUPPORT_H
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarRgbLed.h"
namespace xwalk::hal::test::led
{
    /** @brief Records simulated GPIO operations and optional output failure. */
    struct GpioBackend
    {
            uint32 configureCount{};
            uint32 writeCount{};
            boolean value{};
            atomicboolean failWrites{false};
    };
    /** @brief Records register writes made by simulated PWM objects. */
    struct I2cBackend
    {
            uint32 writeCount{};
    };
    /** @brief Owns the dependency graph required by one RGB LED test. */
    struct RgbFixture
    {
            I2cBackend bus;
            XWalkI2c i2c;
            XWalkPwmTimerState timerState;
            XWalkPwm red;
            XWalkPwm green;
            XWalkPwm blue;
            RgbFixture();
    };
    void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue);
    boolean readGpio(contextpointer context, uint8 pin);
    void writeGpio(contextpointer context, uint8 pin, boolean value);
    void registerGpioInterrupt(contextpointer context,
                               uint8 pin,
                               XWalkGpioEdge edge,
                               uint32 debounceMs,
                               contextpointer handlerContext,
                               gpiointerrupthandler handler);
    void cancelGpioInterrupt(contextpointer context, uint8 pin);
    XWalkGpioCallbacks gpioCallbacks();
    boolean probeI2c(contextpointer context, uint8 address);
    void writeI2cRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
    bytevector readI2c(contextpointer context, uint8 address, size length);
    boolean nearlyEqual(float64 left, float64 right);
} /* namespace xwalk::hal::test::led */
#endif /* XHAL_RPI5CAR_LED_TEST_SUPPORT_H */
