/******************************************************************************
 * @file        xHal_Rpi5CarUtils.h
 * @brief       Declares backend-neutral Robot HAT utility operations.
 *
 * @details
 * Provides colored output, volume, command, executable, network, username,
 * and numeric mapping behavior through caller-owned platform services.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils
 *
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_UTILS_H
#define XHAL_RPI5CAR_UTILS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarUtilsTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkUtils
 * @brief Coordinates generic Robot HAT utilities through injected services.
 *
 * @details
 * Stores a nullable non-owning backend context and a complete callback table.
 * It does not invoke a shell, write a terminal, change volume, inspect a process
 * path, query a network interface, or read environment state directly.
 */
class XWalkUtils final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Nullable non-owning context supplied to every platform callback.
         *
         * @note
         * A non-null object must outlive this utility coordinator. Null is
         * permitted only when every callback supports stateless operation.
         */
        contextpointer backendContextPointer;

        /** @brief Complete non-owning platform callback table copied by value. */
        XWalkUtilsCallbacks callbacks;

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Validates that every required platform callback is non-null.
         *
         * @param[in] backendCallbacks
         * Callback table inspected before any operation is possible.
         *
         * @throws std::invalid_argument
         * If any required callback is null.
         */
        static void validateCallbacks(const XWalkUtilsCallbacks& backendCallbacks);

        /**
         * @brief Validates a terminal-color enumerator before dispatch.
         *
         * @param[in] color
         * Color value to validate.
         *
         * @return
         * Validated color value.
         *
         * @throws std::out_of_range
         * If `color` does not identify a supported terminal color.
         */
        static XWalkUtilityColor validateColor(XWalkUtilityColor color);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a utility coordinator from caller-owned services.
         *
         * @param[in,out] backendContext
         * Nullable non-owning context used by every platform callback.
         *
         * @param[in] backendCallbacks
         * Complete callback table copied by value.
         *
         * @throws std::invalid_argument
         * If any required callback is null.
         */
        XWalkUtils(contextpointer backendContext, const XWalkUtilsCallbacks& backendCallbacks);

        /** @brief Destroys the coordinator without releasing platform resources. */
        ~XWalkUtils();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables copying of the non-owning backend binding. */
        XWalkUtils(const XWalkUtils&) = delete;
        /** @brief Disables copy assignment of the non-owning backend binding. */
        XWalkUtils& operator=(const XWalkUtils&) = delete;
        /** @brief Disables moving because backend context identity is retained. */
        XWalkUtils(XWalkUtils&&) = delete;
        /** @brief Disables move assignment because backend context identity is retained. */
        XWalkUtils& operator=(XWalkUtils&&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Writes one colored record through the output backend.
         *
         * @param[in] message
         * Message forwarded synchronously without encoding changes.
         *
         * @param[in] color
         * Terminal color associated with the message.
         *
         * @param[in] ending
         * Record terminator; newline is used by default.
         *
         * @param[in] flush
         * `true` to request immediate backend flushing; otherwise `false`.
         */
        void printColor(stringview message, XWalkUtilityColor color,
            stringview ending = "\n", boolean flush = false) const;

        /**
         * @brief Writes a white informational record.
         *
         * @param[in] message
         * Message forwarded synchronously.
         *
         * @param[in] ending
         * Record terminator; newline is used by default.
         *
         * @param[in] flush
         * `true` to request immediate flushing; otherwise `false`.
         */
        void info(stringview message, stringview ending = "\n", boolean flush = false) const;

        /**
         * @brief Writes a gray diagnostic record.
         *
         * @param[in] message
         * Message forwarded synchronously.
         *
         * @param[in] ending
         * Record terminator; newline is used by default.
         *
         * @param[in] flush
         * `true` to request immediate flushing; otherwise `false`.
         */
        void debug(stringview message, stringview ending = "\n", boolean flush = false) const;

        /**
         * @brief Writes a yellow warning record.
         *
         * @param[in] message
         * Message forwarded synchronously.
         *
         * @param[in] ending
         * Record terminator; newline is used by default.
         *
         * @param[in] flush
         * `true` to request immediate flushing; otherwise `false`.
         */
        void warning(stringview message, stringview ending = "\n", boolean flush = false) const;

        /**
         * @brief Writes a red error record.
         *
         * @param[in] message
         * Message forwarded synchronously.
         *
         * @param[in] ending
         * Record terminator; newline is used by default.
         *
         * @param[in] flush
         * `true` to request immediate flushing; otherwise `false`.
         */
        void error(stringview message, stringview ending = "\n", boolean flush = false) const;

        /**
         * @brief Applies a clamped system-volume percentage.
         *
         * @param[in] volumePercent
         * Requested percentage; values below zero become zero and values above
         * one hundred become one hundred, preserving Python behavior.
         */
        void setVolume(int32 volumePercent) const;

        /**
         * @brief Executes one application-approved command synchronously.
         *
         * @param[in] command
         * Command text forwarded without modification.
         *
         * @param[in] user
         * Optional backend-defined user name; empty means unspecified.
         *
         * @param[in] group
         * Optional backend-defined group name; empty means unspecified.
         *
         * @return
         * Backend process status and owned combined output.
         *
         * @warning
         * The backend must validate untrusted command text before execution.
         */
        XWalkCommandResult runCommand(stringview command, stringview user = {},
            stringview group = {}) const;

        /**
         * @brief Reports whether one executable can be resolved by the backend.
         *
         * @param[in] command
         * Executable name forwarded synchronously.
         *
         * @return
         * `true` when the executable can be resolved; otherwise `false`.
         */
        boolean commandExists(stringview command) const;

        /**
         * @brief Reports whether one command is installed according to the backend.
         *
         * @param[in] command
         * Executable name forwarded synchronously.
         *
         * @return
         * `true` when the executable can be resolved; otherwise `false`.
         */
        boolean isInstalled(stringview command) const;

        /**
         * @brief Reports whether one executable can be resolved by the backend.
         *
         * @param[in] executable
         * Executable name forwarded synchronously.
         *
         * @return
         * `true` when the executable can be resolved; otherwise `false`.
         */
        boolean checkExecutable(stringview executable) const;

        /**
         * @brief Returns the first IPv4 address found for ordered interfaces.
         *
         * @param[in] interfaces
         * Ordered interface names. Empty names are forwarded to the backend.
         *
         * @return
         * First non-empty IPv4 text, or an empty string when none is available.
         */
        string ipAddress(const stringvector& interfaces) const;

        /**
         * @brief Returns an IPv4 address for one interface.
         *
         * @param[in] interfaceName
         * Interface name forwarded synchronously.
         *
         * @return
         * IPv4 text, or an empty string when the interface has no address.
         */
        string ipAddress(stringview interfaceName) const;

        /**
         * @brief Returns the first address for the default wireless and Ethernet interfaces.
         *
         * @return
         * Address for `wlan0`, then `eth0`, or an empty string when neither is available.
         */
        string ipAddress() const;

        /**
         * @brief Returns the effective application username.
         *
         * @return
         * Backend-provided owned username, including empty when unavailable.
         */
        string username() const;

        /**
         * @brief Maps a finite value linearly between two numeric ranges.
         *
         * @param[in] input
         * Finite value to map; values outside the input range are extrapolated.
         *
         * @param[in] inputMinimum
         * Finite input-range origin.
         *
         * @param[in] inputMaximum
         * Finite input-range end, different from `inputMinimum`.
         *
         * @param[in] outputMinimum
         * Finite output-range origin.
         *
         * @param[in] outputMaximum
         * Finite output-range end.
         *
         * @return
         * Linearly mapped or extrapolated value.
         *
         * @throws std::invalid_argument
         * If an input is not finite or the input range has zero width.
         */
        static float64 mapping(float64 input, float64 inputMinimum,
            float64 inputMaximum, float64 outputMinimum, float64 outputMaximum);
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_UTILS_H */
