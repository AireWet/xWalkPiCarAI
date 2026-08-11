/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345SimulationArguments.h
 * @brief       Declares persistent xWalkAdxl345 trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ADXL345_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_ADXL345_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional ADXL345 trace selector. */
class XWalkAdxl345SimulationArguments final
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
        XWalkAdxl345SimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkAdxl345SimulationArguments();
        XWalkAdxl345SimulationArguments(const XWalkAdxl345SimulationArguments&) = delete;
        XWalkAdxl345SimulationArguments& operator=(const XWalkAdxl345SimulationArguments&) = delete;
        XWalkAdxl345SimulationArguments(XWalkAdxl345SimulationArguments&&) = delete;
        XWalkAdxl345SimulationArguments& operator=(XWalkAdxl345SimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ADXL345_SIMULATION_ARGUMENTS_H */
