/******************************************************************************
 * @file        xControllerSequence.cpp
 * @brief       Implements bounded CLI controller command sequencing.
 *
 * @details
 * Validates the complete command list before execution and returns immediately
 * when the caller-owned controller reports a non-zero status.
 *
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerSequence.h"
#include "xControllerCommands.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent::test
 * @brief Contains host-testable and explicitly selected CLI sequence behavior.
 */
namespace xwalk::agent::test
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Binds one controller used for every sequence command.
 *
 * @param[in] controller
 * Caller-owned controller that must outlive this sequence.
 */
XWalkControllerSequence::XWalkControllerSequence(xwalk::ctrl::XWalkController& controller) noexcept
    : controllerObject(&controller)
{
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Runs every command in order until one reports failure.
 *
 * @param[in] commands
 * One through 32 non-empty command argument sequences.
 *
 * @return
 * Zero when every command succeeds; otherwise the first non-zero command
 * status.
 *
 * @throws std::invalid_argument
 * If the outer sequence or any command is empty.
 *
 * @throws std::out_of_range
 * If more than 32 commands are supplied.
 *
 * @post
 * Commands after the first non-zero result are not executed.
 */
::ctrl::int32 XWalkControllerSequence::run(const controllercommandsequence& commands)
{
    const ::ctrl::boolean commandsEmpty =
        static_cast<::ctrl::boolean>(
            commands.empty());
    if (commandsEmpty)
    {
        XHAL_THROW_INVALID_ARGUMENT("CLI controller sequence must not be empty");
    }
    const ::ctrl::boolean commandsTooLarge =
        static_cast<::ctrl::boolean>(
            commands.size() > XAGENT_RPI5CAR_CONTROLLER_SEQUENCE_MAX_COMMANDS);
    if (commandsTooLarge)
    {
        XHAL_THROW_OUT_OF_RANGE("CLI controller sequence exceeds 32 commands");
    }
    for (const ::ctrl::stringvector& command : commands)
    {
        const ::ctrl::boolean commandEmpty =
            static_cast<::ctrl::boolean>(
                command.empty());
        if (commandEmpty)
        {
            XHAL_THROW_INVALID_ARGUMENT("CLI controller sequence command must not be empty");
        }
    }

    for (const ::ctrl::stringvector& command : commands)
    {
        const ::ctrl::int32 status = xwalk::ctrl::XWALK_runControllerCommand(*controllerObject, command);
        if (status != 0)
        {
            return status;
        }
    }
    return 0;
}

} /* namespace xwalk::agent::test */
