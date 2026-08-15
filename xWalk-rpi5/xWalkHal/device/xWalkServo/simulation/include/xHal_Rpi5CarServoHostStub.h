/******************************************************************************
 * @file        xHal_Rpi5CarServoHostStub.h
 * @brief       Declares the device-free Servo I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkServo Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_SERVO_HOST_STUB_H
#define XHAL_RPI5CAR_SERVO_HOST_STUB_H
#include "xHal_Rpi5CarI2c.h"
namespace xwalk::hal::sim
{
    /** @brief Records Servo PWM register traffic entirely in memory. */
    class XWalkServoHostStub final
    {
        private:
            size writeCountValue{};
            uint8 lastRegisterValue{};
            size lastPayloadSizeValue{};

        public:
            static boolean probe(contextpointer context, uint8 address);
            static void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
            static bytevector read(contextpointer context, uint8 address, size length);
            size writeCount() const noexcept;
            uint8 lastRegister() const noexcept;
            size lastPayloadSize() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_SERVO_HOST_STUB_H */
