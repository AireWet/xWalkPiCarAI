/******************************************************************************
 * @file        xHal_Rpi5CarConfigSimulationArguments.h
 * @brief       Declares xWalkConfig simulation trace-option parsing.
 *
 * @details
 * Owns and validates one optional persistent trace selector for the standalone
 * Config executable.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_CONFIG_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_CONFIG_SIMULATION_ARGUMENTS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::sim
{

/** @brief Validates and applies one optional Config simulation trace selector. */
class XWalkConfigSimulationArguments final
{
    private:
        string traceTargetValue;
        boolean traceEnabledValue;
        boolean traceUpdateRequestedValue;
        boolean validValue;
        boolean helpRequestedValue;

    protected:
        /**
         * @brief Validates one selector target without its operation suffix.
         * @param[in] target Candidate `all`, `RPI`, or `RPI.<digits>` target.
         * @return `true` when the complete target is supported.
         */
        static boolean targetIsValid(stringview target) noexcept;

        /**
         * @brief Parses one complete trace selector or JSON path.
         * @param[in] selector Candidate selector retained only when valid.
         */
        void parseSelector(stringview selector);

    public:
        /**
         * @brief Parses the complete standalone Config command line.
         * @param[in] argumentCount Number of process arguments including the binary name.
         * @param[in] argumentValues Process arguments valid throughout construction.
         */
        XWalkConfigSimulationArguments(int32 argumentCount, charpointer argumentValues[]);
        /** @brief Destroys owned selector state. */
        ~XWalkConfigSimulationArguments();

        XWalkConfigSimulationArguments(const XWalkConfigSimulationArguments&) = delete;
        XWalkConfigSimulationArguments& operator=(const XWalkConfigSimulationArguments&) = delete;
        XWalkConfigSimulationArguments(XWalkConfigSimulationArguments&&) = delete;
        XWalkConfigSimulationArguments& operator=(XWalkConfigSimulationArguments&&) = delete;

        /**
         * @brief Returns whether the command-line shape and selector are valid.
         * @return `true` for no option, help, or one supported trace selector.
         */
        boolean valid() const noexcept;
        /**
         * @brief Returns whether help was requested.
         * @return `true` for `--help` or `-h`; otherwise `false`.
         */
        boolean helpRequested() const noexcept;
        /**
         * @brief Applies and persists the requested trace update.
         * @return `true` when no update is needed or the requested update succeeds.
         */
        boolean applyTraceUpdate() const;
};

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_CONFIG_SIMULATION_ARGUMENTS_H */
