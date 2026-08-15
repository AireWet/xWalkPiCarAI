/******************************************************************************
 * @file        xHal_Rpi5CarAdcTestSupport.h
 * @brief       Declares reusable ADC host-test I2C support.
 * @project     xWalk Firmware
 * @module      xWalkAdc Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ADC_TEST_SUPPORT_H
#define XHAL_RPI5CAR_ADC_TEST_SUPPORT_H
#include "xHal_Rpi5CarI2c.h"
namespace xwalk::hal::test::adc
{
    /** @brief Records ADC I2C traffic and supplies deterministic sample bytes. */
    struct TestBus
    {
            byteset presentAddresses;
            bytevector probes;
            bytevector readBytes{0x0AU, 0xBCU};
            uint8 writeAddress{};
            uint8 writeRegister{};
            bytevector writeData;
            uint8 readAddress{};
            size readLength{};
            mutexhandle mutexValue{};
            conditionvariable conditionValue{};
            fixedarray<uint16, 8U> channelValues{};
            bytevector operationOrder{};
            uint8 selectedCommand{0x17U};
            boolean returnSelectedChannelValue{};
            boolean pauseFirstWrite{};
            boolean firstWriteObserved{};
            boolean contenderReady{};
            boolean releaseFirstWrite{};
    };

    boolean probe(contextpointer context, uint8 address);
    void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
    bytevector read(contextpointer context, uint8 address, size length);
} /* namespace xwalk::hal::test::adc */
#endif /* XHAL_RPI5CAR_ADC_TEST_SUPPORT_H */
