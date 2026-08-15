/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345HostStub.h
 * @brief       Declares the device-free ADXL345 I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ADXL345_HOST_STUB_H
#define XHAL_RPI5CAR_ADXL345_HOST_STUB_H
#include "xHal_Rpi5CarI2c.h"
namespace xwalk::hal::sim
{
    /** @brief Supplies deterministic three-axis samples entirely in memory. */
    class XWalkAdxl345HostStub final
    {
        private:
            size writeCountValue{};
            size registerReadCountValue{};

        public:
            static boolean probe(contextpointer context, uint8 address);
            static void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
            static bytevector read(contextpointer context, uint8 address, size length);
            static bytevector readRegister(contextpointer context, uint8 address, uint8 reg, size length);
            size writeCount() const noexcept;
            size registerReadCount() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ADXL345_HOST_STUB_H */
