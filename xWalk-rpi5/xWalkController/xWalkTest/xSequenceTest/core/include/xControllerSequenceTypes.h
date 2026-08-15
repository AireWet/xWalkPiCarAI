/******************************************************************************
 * @file        xControllerSequenceTypes.h
 * @brief       Declares CLI controller-sequence data types.
 *
 * @details
 * Defines the bounded command collection consumed by the host-testable CLI
 * sequence coordinator.
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

#ifndef XCONTROLLER_SEQUENCE_TYPES_H
#define XCONTROLLER_SEQUENCE_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

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
     * Type definitions
     ******************************************************************************/

    /** @brief Ordered CLI commands, where each inner sequence excludes the executable name. */
    using controllercommandsequence = std::vector<::ctrl::stringvector>;

} /* namespace xwalk::agent::test */

#endif /* XCONTROLLER_SEQUENCE_TYPES_H */
