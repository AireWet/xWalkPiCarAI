/******************************************************************************
 * @file        xHal_Rpi5CarMotorHostStub.h
 * @brief       Declares the device-free Motor I2C and GPIO host stub.
 * @project     xWalk Firmware
 * @module      xWalkMotor Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_MOTOR_HOST_STUB_H
#define XHAL_RPI5CAR_MOTOR_HOST_STUB_H
#include "xHal_Rpi5CarMotor.h"
namespace xwalk::hal::sim
{
    /** @brief Records in-memory motor I2C writes and GPIO direction. */
    class XWalkMotorHostStub final
    {
        private:
            uint32 i2cWriteCountValue{};
            boolean directionValue{};

        public:
            static boolean probe(contextpointer context, uint8 address);
            static void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
            static boolean
            tryWriteRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data) noexcept;
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
            boolean direction() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_MOTOR_HOST_STUB_H */
