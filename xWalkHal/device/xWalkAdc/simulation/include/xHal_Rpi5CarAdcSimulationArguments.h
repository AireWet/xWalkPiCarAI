/******************************************************************************
 * @file        xHal_Rpi5CarAdcSimulationArguments.h
 * @brief       Declares persistent xWalkAdc trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkAdc Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ADC_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_ADC_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional ADC trace selector. */
class XWalkAdcSimulationArguments final
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
        XWalkAdcSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkAdcSimulationArguments();
        XWalkAdcSimulationArguments(const XWalkAdcSimulationArguments&) = delete;
        XWalkAdcSimulationArguments& operator=(const XWalkAdcSimulationArguments&) = delete;
        XWalkAdcSimulationArguments(XWalkAdcSimulationArguments&&) = delete;
        XWalkAdcSimulationArguments& operator=(XWalkAdcSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ADC_SIMULATION_ARGUMENTS_H */
