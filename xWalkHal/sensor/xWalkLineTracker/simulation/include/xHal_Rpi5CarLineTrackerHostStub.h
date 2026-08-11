/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerHostStub.h
 * @brief       Declares the device-free LineTracker I2C host stub.
 * @project     xWalk Firmware
 * @module      xWalkLineTracker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LINE_TRACKER_HOST_STUB_H
#define XHAL_RPI5CAR_LINE_TRACKER_HOST_STUB_H
#include "xHal_Rpi5CarAdc.h"
namespace xwalk::hal::sim
{
/** @brief Supplies deterministic left, middle, and right ADC samples. */
class XWalkLineTrackerHostStub final
{
    private:
        fixedarray<uint16, 3U> samplesValue{200U, 1'000U, 1'000U};
        uint8 commandValue{XHAL_RPI5CAR_ADC_READ_COMMAND};
        uint32 readCountValue{};
    public:
        static boolean probe(contextpointer context, uint8 address);
        static void writeRegister(contextpointer context, uint8 address, uint8 reg,
            const bytevector& data);
        static bytevector read(contextpointer context, uint8 address, size length);
        uint32 readCount() const noexcept;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_LINE_TRACKER_HOST_STUB_H */
