/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerTestSupport.h
 * @brief       Declares reusable line-tracker host-test support.
 * @project     xWalk Firmware
 * @module      xWalkLineTracker Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LINE_TRACKER_TEST_SUPPORT_H
#define XHAL_RPI5CAR_LINE_TRACKER_TEST_SUPPORT_H
#include "xHal_Rpi5CarAdc.h"
namespace xwalk::hal::test::linetracker
{
    /** @brief Supplies one configurable ADC sample for each tracker channel. */
    struct TestBus
    {
            fixedarray<uint16, 3U> samples{1'200U, 800U, 1'000U};
            uint8 command{XHAL_RPI5CAR_ADC_READ_COMMAND};
            uint32 readCount{};
    };
    boolean probe(contextpointer context, uint8 address);
    void writeRegister(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
    bytevector read(contextpointer context, uint8 address, size length);
} /* namespace xwalk::hal::test::linetracker */
#endif /* XHAL_RPI5CAR_LINE_TRACKER_TEST_SUPPORT_H */
