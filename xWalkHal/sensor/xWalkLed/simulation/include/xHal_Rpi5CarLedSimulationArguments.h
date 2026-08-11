/******************************************************************************
 * @file        xHal_Rpi5CarLedSimulationArguments.h
 * @brief       Declares persistent xWalkLed trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkLed Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LED_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_LED_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional LED trace selector. */
class XWalkLedSimulationArguments final
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
        XWalkLedSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkLedSimulationArguments();
        XWalkLedSimulationArguments(const XWalkLedSimulationArguments&) = delete;
        XWalkLedSimulationArguments& operator=(const XWalkLedSimulationArguments&) = delete;
        XWalkLedSimulationArguments(XWalkLedSimulationArguments&&) = delete;
        XWalkLedSimulationArguments& operator=(XWalkLedSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_LED_SIMULATION_ARGUMENTS_H */
