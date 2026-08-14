/******************************************************************************
 * @file        xHal_Rpi5CarSpiSimulationArguments.h
 * @brief       Declares xWalkSpi simulation trace-option parsing.
 *
 * @details
 * Validates the optional trace selector before the simulation opens its
 * build-selected Linux SPI device.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Simulation
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

#ifndef XHAL_RPI5CAR_SPI_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_SPI_SIMULATION_ARGUMENTS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkSpi simulation support.
 */
namespace xwalk::hal::sim
{

/**
 * @class XWalkSpiSimulationArguments
 * @brief Validates and applies one optional simulation trace selector.
 */
class XWalkSpiSimulationArguments final
{
    public:
        /**
         * @brief Parses the complete simulation process argument list.
         * @param[in] argumentCount Number of process arguments, including the binary name.
         * @param[in] argumentValues Non-null process argument array with `argumentCount` entries.
         */
        XWalkSpiSimulationArguments(int32 argumentCount, charpointer argumentValues[]);

        /** @brief Destroys owned selector text without external side effects. */
        ~XWalkSpiSimulationArguments();

        XWalkSpiSimulationArguments(const XWalkSpiSimulationArguments&) = delete;
        XWalkSpiSimulationArguments& operator=(const XWalkSpiSimulationArguments&) = delete;
        XWalkSpiSimulationArguments(XWalkSpiSimulationArguments&&) = delete;
        XWalkSpiSimulationArguments& operator=(XWalkSpiSimulationArguments&&) = delete;

        /** @brief Reports whether the complete process argument list is valid. */
        boolean valid() const noexcept;

        /** @brief Reports whether the caller requested command help. */
        boolean helpRequested() const noexcept;

        /** @brief Applies the parsed trace update to the global trace runtime. */
        boolean applyTraceUpdate() const;

    protected:
        /** @brief Reports whether one selector target is `all` or `RPI.<digits>`. */
        static boolean targetIsValid(stringview target) noexcept;

        /** @brief Parses one selector and commits it only when completely valid. */
        void parseSelector(stringview selector);

    private:
        /** @brief Parsed UID, module, `all`, or JSON path selected by the caller. */
        string traceTargetValue;

        /** @brief Requested trace state when a non-JSON selector is present. */
        boolean traceEnabledValue;

        /** @brief Indicates that the caller supplied one trace selector. */
        boolean traceUpdateRequestedValue;

        /** @brief Indicates that the complete process argument list is valid. */
        boolean validValue;

        /** @brief Indicates that `--help` or `-h` was supplied. */
        boolean helpRequestedValue;
};

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_SPI_SIMULATION_ARGUMENTS_H */
