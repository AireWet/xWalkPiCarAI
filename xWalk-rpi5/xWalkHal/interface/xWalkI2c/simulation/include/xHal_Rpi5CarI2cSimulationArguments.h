/******************************************************************************
 * @file        xHal_Rpi5CarI2cSimulationArguments.h
 * @brief       Declares xWalkI2c simulation trace-option parsing.
 *
 * @details
 * Validates the optional trace selector before the simulation opens its
 * build-selected Linux I2C device.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-09
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_I2C_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_I2C_SIMULATION_ARGUMENTS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkI2c simulation support.
 */
namespace xwalk::hal::sim
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkI2cSimulationArguments
     * @brief Validates and applies one optional simulation trace selector.
     *
     * @details
     * Accepts no option, `--help`, `-h`, or `--trace` followed by one
     * `RPI.<digits>` or `all` selector ending in `.enable` or `.disable`. The
     * object owns parsed text and does not retain the caller's argument pointers.
     */
    class XWalkI2cSimulationArguments final
    {
        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Parses the complete simulation process argument list.
             * @param[in] argumentCount Number of process arguments, including the binary name.
             * @param[in] argumentValues Non-null process argument array with `argumentCount` entries.
             */
            XWalkI2cSimulationArguments(int32 argumentCount, charpointer argumentValues[]);

            /** @brief Destroys owned selector text without external side effects. */
            ~XWalkI2cSimulationArguments();

            XWalkI2cSimulationArguments(const XWalkI2cSimulationArguments&) = delete;
            XWalkI2cSimulationArguments& operator=(const XWalkI2cSimulationArguments&) = delete;
            XWalkI2cSimulationArguments(XWalkI2cSimulationArguments&&) = delete;
            XWalkI2cSimulationArguments& operator=(XWalkI2cSimulationArguments&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Reports whether the complete process argument list is valid.
             * @return `true` for no option, help, or one supported `--trace` selector.
             */
            boolean valid() const noexcept;

            /**
             * @brief Reports whether the caller requested command help.
             * @return `true` for `--help` or `-h`; otherwise `false`.
             */
            boolean helpRequested() const noexcept;

            /**
             * @brief Applies the parsed trace update to the configured global trace runtime.
             * @return `true` when no update is needed or registry application succeeds.
             */
            boolean applyTraceUpdate() const;

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Reports whether one selector target is `all` or `RPI.<digits>`. */
            static boolean targetIsValid(stringview target) noexcept;

            /** @brief Parses one selector and commits it only when completely valid. */
            void parseSelector(stringview selector);

        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Parsed UID, or `all` when every scanner-known UID is selected. */
            string traceTargetValue;

            /** @brief Requested trace state when a selector is present. */
            boolean traceEnabledValue;

            /** @brief Indicates that the caller supplied one trace selector. */
            boolean traceUpdateRequestedValue;

            /** @brief Indicates that the complete process argument list is valid. */
            boolean validValue;

            /** @brief Indicates that `--help` or `-h` was supplied. */
            boolean helpRequestedValue;
    };

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_I2C_SIMULATION_ARGUMENTS_H */
