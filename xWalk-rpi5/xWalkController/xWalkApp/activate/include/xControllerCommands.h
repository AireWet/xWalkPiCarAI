/******************************************************************************
 * @file        xControllerCommands.h
 * @brief       Declares the xWalk Controller application command functions.
 *
 * @details
 * Provides application-owned free functions for command dispatch and generated
 * command help without adding application entry behavior to the handler class.
 *
 * @project     xWalk Firmware
 * @module      xWalkApp
 *
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XCONTROLLER_COMMANDS_H
#define XCONTROLLER_COMMANDS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xWalkControllerConfigTypes.h"
#include "xController.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

    /******************************************************************************
     * Function declarations
     ******************************************************************************/

    /**
     * @brief Parses process arguments shared by host and hardware applications.
     *
     * @param[in] argumentCount Number of process arguments including the executable name.
     * @param[in] arguments Non-owning process argument array.
     * @param[in] defaultConfig Default deployment and packaged-resource paths.
     * @param[out] applicationArguments Validated paths and remaining command arguments.
     *
     * @return
     * `true` when every application-global option is complete, path values are
     * non-empty, and trace selectors are valid; otherwise `false`.
     */
    ::ctrl::boolean xWalkParseControllerApplicationArguments(::ctrl::int32 argumentCount,
                                                             ::ctrl::charpointer arguments[],
                                                             const XWalkAppConfig& defaultConfig,
                                                             XWalkControllerApplicationArguments& applicationArguments);

    /**
     * @brief Reports whether one parsed command requests generated help.
     *
     * @param[in] arguments Command arguments after global-option parsing.
     *
     * @return
     * `true` only for one `help`, `-h`, or `--help` argument.
     */
    ::ctrl::boolean XWALK_isControllerHelpRequest(const ::ctrl::stringvector& arguments) noexcept;

    /**
     * @brief Parses one non-empty top-level command into the shared request type.
     *
     * @param[in] arguments Command arguments excluding the executable name.
     *
     * @return
     * Owned arguments and the recognized command, or `Unknown` for an unsupported
     * command name.
     *
     * @throws std::invalid_argument
     * The argument list is empty.
     */
    XWalkControllerCommandRequest XWALK_parseControllerCommand(const ::ctrl::stringvector& arguments);

    /**
     * @brief Executes one application command through a configured controller.
     *
     * @param[in,out] controller Controller whose non-owning dependencies and
     * callbacks remain valid for the complete call.
     * @param[in] arguments Command arguments excluding the executable name.
     *
     * @return
     * Zero on success, two for a command-level failure, or three when the selected
     * backend is unavailable.
     *
     * @throws std::invalid_argument
     * The argument list is empty or the selected command syntax is invalid.
     *
     * @post
     * A PiCar-X command has performed command-scope emergency-stop cleanup.
     */
    ::ctrl::int32 XWALK_runControllerCommand(XWalkController& controller, const ::ctrl::stringvector& arguments);

    /**
     * @brief Returns the generated Linux-style command help.
     *
     * @return
     * Owned multi-line help text describing commands, options, and examples.
     */
    ::ctrl::string XWALK_controllerUsage();

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_COMMANDS_H */
