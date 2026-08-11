/******************************************************************************
 * @file        xHal_Rpi5CarPwmSimulationArguments.h
 * @brief       Declares persistent xWalkPwm trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_PWM_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_PWM_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional PWM trace selector. */
class XWalkPwmSimulationArguments final
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
        XWalkPwmSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkPwmSimulationArguments();
        XWalkPwmSimulationArguments(const XWalkPwmSimulationArguments&) = delete;
        XWalkPwmSimulationArguments& operator=(const XWalkPwmSimulationArguments&) = delete;
        XWalkPwmSimulationArguments(XWalkPwmSimulationArguments&&) = delete;
        XWalkPwmSimulationArguments& operator=(XWalkPwmSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_PWM_SIMULATION_ARGUMENTS_H */
