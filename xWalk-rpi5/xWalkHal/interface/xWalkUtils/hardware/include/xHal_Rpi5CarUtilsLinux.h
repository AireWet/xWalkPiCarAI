/******************************************************************************
 * @file        xHal_Rpi5CarUtilsLinux.h
 * @brief       Declares the Linux platform backend for xWalk utilities.
 *
 * @details
 * Provides callback-compatible terminal, mixer, process, executable, network,
 * user, and standard-error operations for Raspberry Pi Linux applications.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils Linux Backend
 *
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_UTILS_LINUX_H
#define XHAL_RPI5CAR_UTILS_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarStderrGuard.h"
#include "xHal_Rpi5CarUtils.h"

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
     * @class XWalkUtilsLinux
     * @brief Supplies Raspberry Pi Linux services to `XWalkUtils`.
     *
     * @details
     * Owns no persistent operating-system resource. The application creates this
     * backend before `XWalkUtils` and any `XWalkStderrGuard` bound to its callbacks.
     * Command execution deliberately preserves the upstream shell-text contract;
     * callers must therefore accept only trusted or independently validated text.
     */
    class XWalkUtilsLinux final
    {
        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Returns the ANSI Select Graphic Rendition code for a utility color.
             *
             * @param[in] color
             * Supported terminal color.
             *
             * @return
             * Non-owning static ANSI code text.
             *
             * @throws std::out_of_range
             * If `color` is not supported.
             */
            static cstring colorCode(XWalkUtilityColor color);

            /**
             * @brief Writes a complete byte sequence to one Linux descriptor.
             *
             * @param[in] descriptor
             * Open writable Linux descriptor.
             *
             * @param[in] bytes
             * Byte view written completely before return.
             *
             * @throws std::runtime_error
             * If the descriptor write cannot complete.
             */
            static void writeAll(int32 descriptor, stringview bytes);

            /**
             * @brief Checks whether a path identifies an accessible executable file.
             *
             * @param[in] path
             * Non-empty Linux filesystem path.
             *
             * @return
             * `true` when the path is a regular executable file; otherwise `false`.
             */
            static boolean isExecutableFile(const string& path);

            /**
             * @brief Bridges a utility-output callback to its backend object.
             *
             * @param[in,out] context
             * Non-null pointer to a live `XWalkUtilsLinux` object.
             *
             * @param[in] color
             * Supported terminal color.
             *
             * @param[in] message
             * Message view valid for this synchronous call.
             *
             * @param[in] ending
             * Record-ending view valid for this synchronous call.
             *
             * @param[in] flush
             * Flush request; Linux descriptor writes complete synchronously.
             */
            static void outputCallback(
                contextpointer context, XWalkUtilityColor color, stringview message, stringview ending, boolean flush);

            /**
             * @brief Bridges a volume callback to its backend object.
             *
             * @param[in,out] context
             * Non-null pointer to a live `XWalkUtilsLinux` object.
             *
             * @param[in] volumePercent
             * Volume in the inclusive range zero through one hundred percent.
             */
            static void volumeCallback(contextpointer context, uint8 volumePercent);

            /**
             * @brief Bridges a command callback to its backend object.
             *
             * @param[in,out] context
             * Non-null pointer to a live `XWalkUtilsLinux` object.
             *
             * @param[in] command
             * Trusted shell command text valid for this synchronous call.
             *
             * @param[in] user
             * Optional Linux user name.
             *
             * @param[in] group
             * Optional Linux group name.
             *
             * @return
             * Process exit status and combined output.
             */
            static XWalkCommandResult
            commandCallback(contextpointer context, stringview command, stringview user, stringview group);

            /**
             * @brief Bridges an executable lookup to its backend object.
             *
             * @param[in,out] context
             * Non-null pointer to a live `XWalkUtilsLinux` object.
             *
             * @param[in] executable
             * Executable name or path valid for this synchronous call.
             *
             * @return
             * `true` when an executable file is accessible; otherwise `false`.
             */
            static boolean executableCallback(contextpointer context, stringview executable);

            /**
             * @brief Bridges an IPv4 lookup to its backend object.
             *
             * @param[in,out] context
             * Non-null pointer to a live `XWalkUtilsLinux` object.
             *
             * @param[in] interfaceName
             * Linux network-interface name valid for this synchronous call.
             *
             * @return
             * Owned dotted-decimal IPv4 text, or an empty string when unavailable.
             */
            static string ipCallback(contextpointer context, stringview interfaceName);

            /**
             * @brief Bridges a username lookup to its backend object.
             *
             * @param[in,out] context
             * Non-null pointer to a live `XWalkUtilsLinux` object.
             *
             * @return
             * Owned effective application username, or an empty string when unavailable.
             */
            static string usernameCallback(contextpointer context);

            /**
             * @brief Bridges standard-error suppression to its backend object.
             *
             * @param[in,out] context
             * Non-null pointer to a live `XWalkUtilsLinux` object.
             *
             * @return
             * Owned duplicate of the prior standard-error descriptor.
             */
            static int32 redirectCallback(contextpointer context);

            /**
             * @brief Bridges standard-error restoration to its backend object.
             *
             * @param[in,out] context
             * Non-null pointer to a live `XWalkUtilsLinux` object.
             *
             * @param[in] restoreToken
             * Descriptor returned by the paired redirect callback.
             */
            static void restoreCallback(contextpointer context, int32 restoreToken) noexcept;

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /** @brief Constructs a stateless Linux utility backend. */
            XWalkUtilsLinux() = default;

            /** @brief Destroys the backend after all bound consumers are destroyed. */
            ~XWalkUtilsLinux() = default;

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables copying because callback contexts retain object identity. */
            XWalkUtilsLinux(const XWalkUtilsLinux&) = delete;
            /** @brief Disables copy assignment because callback contexts retain object identity. */
            XWalkUtilsLinux& operator=(const XWalkUtilsLinux&) = delete;
            /** @brief Disables moving because callback contexts retain object identity. */
            XWalkUtilsLinux(XWalkUtilsLinux&&) = delete;
            /** @brief Disables move assignment because callback contexts retain object identity. */
            XWalkUtilsLinux& operator=(XWalkUtilsLinux&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Returns the complete callback table for `XWalkUtils`.
             *
             * @return
             * Callback table whose context must point to this live backend.
             */
            XWalkUtilsCallbacks utilityCallbacks() const noexcept;

            /**
             * @brief Returns the Linux standard-error redirect callback.
             *
             * @return
             * Callback whose context must point to this live backend.
             */
            utilityredirectcallback stderrRedirectCallback() const noexcept;

            /**
             * @brief Returns the Linux standard-error restore callback.
             *
             * @return
             * Non-throwing callback whose context must point to this live backend.
             */
            utilityrestorecallback stderrRestoreCallback() const noexcept;

            /**
             * @brief Writes ANSI-colored text to standard output.
             *
             * @param[in] color
             * Supported terminal color.
             *
             * @param[in] message
             * Message bytes written between the ANSI prefix and reset sequence.
             *
             * @param[in] ending
             * Bytes appended after the reset sequence.
             *
             * @param[in] flush
             * Flush request retained for callback compatibility; descriptor writes
             * are synchronous and have no user-space stream buffer.
             *
             * @throws std::out_of_range
             * If `color` is not supported.
             *
             * @throws std::runtime_error
             * If standard output cannot accept the complete record.
             */
            void writeOutput(XWalkUtilityColor color, stringview message, stringview ending, boolean flush) const;

            /**
             * @brief Applies the PCM playback volume through `amixer`.
             *
             * @param[in] volumePercent
             * Volume in the inclusive range zero through one hundred percent.
             *
             * @throws std::runtime_error
             * If `amixer` does not complete successfully.
             */
            void setSystemVolume(uint8 volumePercent) const;

            /**
             * @brief Executes trusted command text through the system shell.
             *
             * @param[in] command
             * Trusted command text forwarded without modification.
             *
             * @param[in] user
             * Optional existing Linux user applied in the child process.
             *
             * @param[in] group
             * Optional existing Linux group applied in the child process.
             *
             * @return
             * Normalized exit status and owned combined standard output and error.
             *
             * @throws std::invalid_argument
             * If a requested user or group cannot be resolved.
             *
             * @throws std::runtime_error
             * If pipe, process, read, or wait operations fail.
             *
             * @warning
             * Shell metacharacters are interpreted. Validate every untrusted input
             * before building or forwarding command text.
             */
            XWalkCommandResult runCommandProcess(stringview command, stringview user = {}, stringview group = {}) const;

            /**
             * @brief Checks whether one executable path or name is accessible.
             *
             * @param[in] executable
             * Absolute path, relative path containing a slash, or bare executable name.
             *
             * @return
             * `true` when an executable file is accessible; otherwise `false`.
             */
            boolean executableExists(stringview executable) const;

            /**
             * @brief Queries the first IPv4 address assigned to an interface.
             *
             * @param[in] interfaceName
             * Exact Linux interface name.
             *
             * @return
             * Owned dotted-decimal IPv4 text, or an empty string when unavailable.
             */
            string interfaceIpv4(stringview interfaceName) const;

            /**
             * @brief Returns the effective application username.
             *
             * @return
             * `SUDO_USER`, then `LOGNAME`, then the effective-user database name,
             * or an empty string when none is available.
             */
            string effectiveUsername() const;

            /**
             * @brief Redirects standard error to `/dev/null`.
             *
             * @return
             * Owned duplicate of the prior standard-error descriptor.
             *
             * @throws std::runtime_error
             * If the null device or descriptor operations fail.
             */
            int32 redirectStandardError() const;

            /**
             * @brief Restores standard error and closes the restore token.
             *
             * @param[in] restoreToken
             * Descriptor returned by `redirectStandardError()`.
             *
             * @note
             * Descriptor errors cannot be reported from the guard destructor.
             */
            void restoreStandardError(int32 restoreToken) const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_UTILS_LINUX_H */
