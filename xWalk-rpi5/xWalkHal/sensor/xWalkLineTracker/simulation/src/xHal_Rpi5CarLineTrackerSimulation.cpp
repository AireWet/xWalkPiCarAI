/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerSimulation.cpp
 * @brief       Implements the device-free xWalkLineTracker simulation.
 * @project     xWalk Firmware
 * @module      xWalkLineTracker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarLineTrackerSimulation.h"
#include "xHal_Rpi5CarGrayscaleModule.h"
#include "xHal_Rpi5CarLineTracker.h"
#include "xHal_Rpi5CarLineTrackerHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    int32 runLineTrackerSimulation()
    {
        XWalkLineTrackerHostStub backend;
        XWalkI2c i2c(&backend,
                     &XWalkLineTrackerHostStub::probe,
                     &XWalkLineTrackerHostStub::writeRegister,
                     &XWalkLineTrackerHostStub::read);
        XWalkAdc left(i2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkAdc middle(i2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkAdc right(i2c, 2U, XHAL_RPI5CAR_ADC_ADDRESS_1);
        XWalkGrayscaleModule grayscale(left, middle, right);
        XWalkLineTracker tracker(left, middle, right);
        const linetrackervalues raw = grayscale.read();
        const float64 position = tracker.getLinePosition();
        const boolean valid =
            (raw == linetrackervalues({200, 1'000, 1'000})) && (position == -0.53) && (backend.readCount() == 6U);
        XWALK_HAL_TRACE_UID0(RPI .240, "xWalkLineTracker host simulation completed");
        return valid ? 0 : 1;
    }
} /* namespace xwalk::hal::sim */
