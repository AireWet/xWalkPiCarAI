/******************************************************************************
 * @file        xControllerSequence.h
 * @brief       Declares bounded CLI controller command sequencing.
 *
 * @details
 * Coordinates an ordered list of commands through one caller-owned controller
 * and stops when a command reports a non-zero status.
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

#ifndef XCONTROLLER_SEQUENCE_H
#define XCONTROLLER_SEQUENCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xController.h"
#include "xControllerSequenceTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent::test
 * @brief Contains host-testable and explicitly selected CLI sequence behavior.
 */
namespace xwalk::agent::test
{

    /******************************************************************************
     * Constants
     ******************************************************************************/

    /** @brief Maximum commands accepted by one bounded CLI sequence. */
    inline constexpr ::ctrl::size XAGENT_RPI5CAR_CONTROLLER_SEQUENCE_MAX_COMMANDS{32U};

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkControllerSequence
     * @brief Runs bounded commands through one caller-owned CLI controller.
     *
     * @details
     * Stores a non-owning controller pointer that remains non-null after
     * construction. The controller must outlive this sequence object.
     */
    class XWalkControllerSequence
    {
        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Binds one controller used for every sequence command.
             *
             * @param[in] controller
             * Caller-owned controller that must outlive this sequence.
             */
            explicit XWalkControllerSequence(xwalk::ctrl::XWalkController& controller) noexcept;

            /** @brief Releases sequence state without releasing the controller. */
            ~XWalkControllerSequence() = default;

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            XWalkControllerSequence(const XWalkControllerSequence&) = delete;
            XWalkControllerSequence(XWalkControllerSequence&&) = delete;
            XWalkControllerSequence& operator=(const XWalkControllerSequence&) = delete;
            XWalkControllerSequence& operator=(XWalkControllerSequence&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

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
            ::ctrl::int32 run(const controllercommandsequence& commands);

        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Non-owning, non-null controller pointer.
             *
             * @details
             * The caller retains ownership and must keep the controller alive for this
             * sequence object's complete lifetime.
             */
            xwalk::ctrl::XWalkController* controllerObject;
    };

} /* namespace xwalk::agent::test */

#endif /* XCONTROLLER_SEQUENCE_H */
