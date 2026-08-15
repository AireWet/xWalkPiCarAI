/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerHostStub.h
 * @brief       Declares the device-free Buzzer GPIO and I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkBuzzer Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_BUZZER_HOST_STUB_H
#define XHAL_RPI5CAR_BUZZER_HOST_STUB_H
#include "xHal_Rpi5CarBuzzer.h"
namespace xwalk::hal::sim
{
    /** @brief Records in-memory active GPIO state and passive PWM I2C writes. */
    class XWalkBuzzerHostStub final
    {
        private:
            uint32 i2cWriteCountValue{};
            boolean gpioValue{};

        public:
            static boolean probe(contextpointer context, uint8 address);
            static void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
            static bytevector read(contextpointer context, uint8 address, size length);
            static void configureGpio(
                contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue);
            static boolean readGpio(contextpointer context, uint8 pin);
            static void writeGpio(contextpointer context, uint8 pin, boolean value);
            static void interruptGpio(contextpointer context,
                                      uint8 pin,
                                      XWalkGpioEdge edge,
                                      uint32 debounceMs,
                                      contextpointer handlerContext,
                                      gpiointerrupthandler handler);
            static void cancelGpioInterrupt(contextpointer context, uint8 pin);
            static XWalkGpioCallbacks gpioCallbacks();
            uint32 i2cWriteCount() const noexcept;
            boolean gpioState() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_BUZZER_HOST_STUB_H */
