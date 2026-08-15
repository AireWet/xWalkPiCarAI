/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerSimulationArguments.h
 * @brief       Declares persistent LineTracker trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkLineTracker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LINE_TRACKER_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_LINE_TRACKER_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Validates and applies one optional LineTracker trace selector. */
    class XWalkLineTrackerSimulationArguments final
    {
        private:
            string traceTargetValue;
            boolean traceEnabledValue;
            boolean traceUpdateRequestedValue;
            boolean validValue;
            boolean helpRequestedValue;

        protected:
            static boolean targetIsValid(stringview target) noexcept;
            void parseSelector(stringview selector);

        public:
            XWalkLineTrackerSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
            ~XWalkLineTrackerSimulationArguments();
            XWalkLineTrackerSimulationArguments(const XWalkLineTrackerSimulationArguments&) = delete;
            XWalkLineTrackerSimulationArguments& operator=(const XWalkLineTrackerSimulationArguments&) = delete;
            XWalkLineTrackerSimulationArguments(XWalkLineTrackerSimulationArguments&&) = delete;
            XWalkLineTrackerSimulationArguments& operator=(XWalkLineTrackerSimulationArguments&&) = delete;
            boolean valid() const noexcept;
            boolean helpRequested() const noexcept;
            boolean applyTraceUpdate() const;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_LINE_TRACKER_SIMULATION_ARGUMENTS_H */
