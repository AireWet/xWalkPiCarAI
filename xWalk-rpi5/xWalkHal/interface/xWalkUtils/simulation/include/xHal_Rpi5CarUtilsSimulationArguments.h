/******************************************************************************
 * @file        xHal_Rpi5CarUtilsSimulationArguments.h
 * @brief       Declares xWalkUtils simulation trace-option parsing.
 * @details     Owns and validates one optional persistent trace selector.
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_UTILS_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_UTILS_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional Utils simulation trace selector. */
class XWalkUtilsSimulationArguments final
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
        XWalkUtilsSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkUtilsSimulationArguments();
        XWalkUtilsSimulationArguments(const XWalkUtilsSimulationArguments&) = delete;
        XWalkUtilsSimulationArguments& operator=(const XWalkUtilsSimulationArguments&) = delete;
        XWalkUtilsSimulationArguments(XWalkUtilsSimulationArguments&&) = delete;
        XWalkUtilsSimulationArguments& operator=(XWalkUtilsSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_UTILS_SIMULATION_ARGUMENTS_H */
