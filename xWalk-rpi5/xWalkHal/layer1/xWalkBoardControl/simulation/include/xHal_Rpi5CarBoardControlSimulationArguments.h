/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlSimulationArguments.h
 * @brief       Declares persistent BoardControl trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_BOARD_CONTROL_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_BOARD_CONTROL_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional BoardControl trace selector. */
class XWalkBoardControlSimulationArguments final
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
        XWalkBoardControlSimulationArguments(int32 count, charpointer values[]);
        ~XWalkBoardControlSimulationArguments();
        XWalkBoardControlSimulationArguments(const XWalkBoardControlSimulationArguments&) = delete;
        XWalkBoardControlSimulationArguments& operator=(
            const XWalkBoardControlSimulationArguments&) = delete;
        XWalkBoardControlSimulationArguments(XWalkBoardControlSimulationArguments&&) = delete;
        XWalkBoardControlSimulationArguments& operator=(
            XWalkBoardControlSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_BOARD_CONTROL_SIMULATION_ARGUMENTS_H */
