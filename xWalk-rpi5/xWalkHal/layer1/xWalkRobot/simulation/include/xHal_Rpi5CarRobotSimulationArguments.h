/******************************************************************************
 * @file        xHal_Rpi5CarRobotSimulationArguments.h
 * @brief       Declares persistent xWalkRobot trace argument parsing.
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_ROBOT_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_ROBOT_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Validates and applies one optional Robot trace selector. */
    class XWalkRobotSimulationArguments final
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
            XWalkRobotSimulationArguments(int32 count, charpointer values[]);
            ~XWalkRobotSimulationArguments();
            XWalkRobotSimulationArguments(const XWalkRobotSimulationArguments&) = delete;
            XWalkRobotSimulationArguments& operator=(const XWalkRobotSimulationArguments&) = delete;
            XWalkRobotSimulationArguments(XWalkRobotSimulationArguments&&) = delete;
            XWalkRobotSimulationArguments& operator=(XWalkRobotSimulationArguments&&) = delete;
            boolean valid() const noexcept;
            boolean helpRequested() const noexcept;
            boolean applyTraceUpdate() const;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_ROBOT_SIMULATION_ARGUMENTS_H */
