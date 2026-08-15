/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonSimulationArguments.h
 * @brief       Declares persistent xWalkUserButton trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkUserButton Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_USER_BUTTON_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_USER_BUTTON_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Validates and applies one optional UserButton trace selector. */
    class XWalkUserButtonSimulationArguments final
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
            XWalkUserButtonSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
            ~XWalkUserButtonSimulationArguments();
            XWalkUserButtonSimulationArguments(const XWalkUserButtonSimulationArguments&) = delete;
            XWalkUserButtonSimulationArguments& operator=(const XWalkUserButtonSimulationArguments&) = delete;
            XWalkUserButtonSimulationArguments(XWalkUserButtonSimulationArguments&&) = delete;
            XWalkUserButtonSimulationArguments& operator=(XWalkUserButtonSimulationArguments&&) = delete;
            boolean valid() const noexcept;
            boolean helpRequested() const noexcept;
            boolean applyTraceUpdate() const;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_USER_BUTTON_SIMULATION_ARGUMENTS_H */
