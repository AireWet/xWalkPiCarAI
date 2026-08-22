/******************************************************************************
 * @file        xControllerRpi.h
 * @brief       Declares Raspberry Pi process initialization.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#ifndef XCONTROLLER_RPI_H
#define XCONTROLLER_RPI_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::ctrl
{

    /** @brief Identifies the one top-level action selected from parsed arguments. */
    typedef enum XWalkRpiAction
    {
        XWALK_RPI_CONFIG_ACTION = 0,
        XWALK_RPI_TRACE_ACTION,
        XWALK_RPI_HELP_ACTION,
        XWALK_RPI_COMMAND_ACTION
    } XWalkRpiAction;

    /** @brief Initializes the CMake-selected Raspberry Pi process environment. */
    ::ctrl::int32 XWALK_initRpi() noexcept;

    /** @brief Parses and sends one Raspberry Pi application action. */
    ::ctrl::int32 xWalkRunRpiApplication(::ctrl::int32 argumentCount, ::ctrl::charpointer arguments[]);

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_RPI_H */
