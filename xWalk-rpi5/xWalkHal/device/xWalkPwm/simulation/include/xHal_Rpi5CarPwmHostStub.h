/******************************************************************************
 * @file        xHal_Rpi5CarPwmHostStub.h
 * @brief       Declares the device-free PWM I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_PWM_HOST_STUB_H
#define XHAL_RPI5CAR_PWM_HOST_STUB_H
#include "xHal_Rpi5CarI2c.h"
namespace xwalk::hal::sim
{
    /** @brief Records PWM I2C traffic entirely in memory. */
    class XWalkPwmHostStub final
    {
        private:
            size writeCountValue{};
            uint8 lastRegisterValue{};
            size lastPayloadSizeValue{};
            uint8 lastHighByteValue{};
            uint8 lastLowByteValue{};

        public:
            static boolean probe(contextpointer context, uint8 address);
            static void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
            static boolean
            tryWriteRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data) noexcept;
            static bytevector read(contextpointer context, uint8 address, size length);
            size writeCount() const noexcept;
            uint8 lastRegister() const noexcept;
            size lastPayloadSize() const noexcept;
            uint8 lastHighByte() const noexcept;
            uint8 lastLowByte() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_PWM_HOST_STUB_H */
