/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345TestSupport.h
 * @brief       Declares reusable ADXL345 host-test I2C support.
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ADXL345_TEST_SUPPORT_H
#define XHAL_RPI5CAR_ADXL345_TEST_SUPPORT_H
#include "xHal_Rpi5CarI2c.h"
namespace xwalk::hal::test::adxl345
{
    /** @brief Records ADXL345 traffic and supplies register responses. */
    struct TestBus
    {
            bytevectorvector responses;
            size responseIndex{};
            uint32 formatWriteCount{};
            uint32 powerWriteCount{};
            uint32 registerReadCount{};
            uint8 lastAddress{};
            uint8 lastRegister{};
            size lastLength{};
    };
    boolean probe(contextpointer context, uint8 address);
    void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
    bytevector read(contextpointer context, uint8 address, size length);
    bytevector readRegister(contextpointer context, uint8 address, uint8 reg, size length);
} /* namespace xwalk::hal::test::adxl345 */
#endif /* XHAL_RPI5CAR_ADXL345_TEST_SUPPORT_H */
