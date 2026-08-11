/******************************************************************************
 * @file        xHal_Rpi5CarCameraSimulationArguments.h
 * @brief       Declares persistent xWalkCamera trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkCamera Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_CAMERA_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_CAMERA_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional xWalkCamera trace selector. */
class XWalkCameraSimulationArguments final
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
        XWalkCameraSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        ~XWalkCameraSimulationArguments();
        XWalkCameraSimulationArguments(const XWalkCameraSimulationArguments&) = delete;
        XWalkCameraSimulationArguments& operator=(const XWalkCameraSimulationArguments&) = delete;
        XWalkCameraSimulationArguments(XWalkCameraSimulationArguments&&) = delete;
        XWalkCameraSimulationArguments& operator=(XWalkCameraSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_CAMERA_SIMULATION_ARGUMENTS_H */
