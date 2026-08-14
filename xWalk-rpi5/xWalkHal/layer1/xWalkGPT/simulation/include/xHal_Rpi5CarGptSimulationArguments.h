/******************************************************************************
 * @file        xHal_Rpi5CarGptSimulationArguments.h
 * @brief       Declares persistent xWalkGPT trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_GPT_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_GPT_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional GPT trace selector. */
class XWalkGptSimulationArguments final
{
    private:
        string targetValue; boolean enabledValue; boolean updateValue;
        boolean validValue; boolean helpValue;
    protected:
        static boolean targetIsValid(stringview target) noexcept;
        void parseSelector(stringview selector);
    public:
        XWalkGptSimulationArguments(int32 count, charpointer values[]);
        ~XWalkGptSimulationArguments();
        XWalkGptSimulationArguments(const XWalkGptSimulationArguments&) = delete;
        XWalkGptSimulationArguments& operator=(const XWalkGptSimulationArguments&) = delete;
        XWalkGptSimulationArguments(XWalkGptSimulationArguments&&) = delete;
        XWalkGptSimulationArguments& operator=(XWalkGptSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_GPT_SIMULATION_ARGUMENTS_H */
