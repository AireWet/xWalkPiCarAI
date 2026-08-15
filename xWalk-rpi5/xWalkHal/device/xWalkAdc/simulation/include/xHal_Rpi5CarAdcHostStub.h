/******************************************************************************
 * @file        xHal_Rpi5CarAdcHostStub.h
 * @brief       Declares the device-free ADC I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkAdc Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ADC_HOST_STUB_H
#define XHAL_RPI5CAR_ADC_HOST_STUB_H
#include "xHal_Rpi5CarI2c.h"
namespace xwalk::hal::sim
{
    /** @brief Supplies deterministic ADC traffic without opening an I2C device. */
    class XWalkAdcHostStub final
    {
        private:
            size probeCountValue{};
            size writeCountValue{};
            uint8 commandValue{};

        public:
            static boolean probe(contextpointer context, uint8 address);
            static void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
            static bytevector read(contextpointer context, uint8 address, size length);
            size probeCount() const noexcept;
            size writeCount() const noexcept;
            uint8 command() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ADC_HOST_STUB_H */
