/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlHostStub.h
 * @brief       Declares device-free BoardControl simulation adapters.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_BOARD_CONTROL_HOST_STUB_H
#define XHAL_RPI5CAR_BOARD_CONTROL_HOST_STUB_H
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarFirmwareInfo.h"
namespace xwalk::hal::sim
{
    /** @brief Provides deterministic GPIO, ADC, firmware, and speaker-prime callbacks. */
    class XWalkBoardControlHostStub final
    {
        private:
            boolean gpioValueValue{};
            uint32 primeCountValue{};

        public:
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
            static void cancelGpio(contextpointer context, uint8 pin);
            static boolean probeI2c(contextpointer context, uint8 address);
            static void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
            static bytevector readI2c(contextpointer context, uint8 address, size length);
            static bytevector readRegisterI2c(contextpointer context, uint8 address, uint8 reg, size length);
            static void primeSpeaker(contextpointer context, uint32 durationMs);
            static XWalkGpioCallbacks gpioCallbacks();
            boolean gpioValue() const noexcept;
            uint32 primeCount() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_BOARD_CONTROL_HOST_STUB_H */
