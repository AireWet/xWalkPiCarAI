/******************************************************************************
 * @file        xHal_Rpi5CarMotorSimulationArguments.h
 * @brief       Declares persistent xWalkMotor trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkMotor Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_MOTOR_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_MOTOR_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional Motor trace selector. */
class XWalkMotorSimulationArguments final
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
        XWalkMotorSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkMotorSimulationArguments();
        XWalkMotorSimulationArguments(const XWalkMotorSimulationArguments&) = delete;
        XWalkMotorSimulationArguments& operator=(const XWalkMotorSimulationArguments&) = delete;
        XWalkMotorSimulationArguments(XWalkMotorSimulationArguments&&) = delete;
        XWalkMotorSimulationArguments& operator=(XWalkMotorSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_MOTOR_SIMULATION_ARGUMENTS_H */
