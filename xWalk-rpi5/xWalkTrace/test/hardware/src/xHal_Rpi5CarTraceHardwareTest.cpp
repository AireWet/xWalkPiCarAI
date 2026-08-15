/******************************************************************************
 * @file        xHal_Rpi5CarTraceHardwareTest.cpp
 * @brief       Compile-checks trace composition for a target configuration.
 *
 * @details
 * Constructs the hardware-independent trace interface with a synchronous
 * application callback. The executable performs no physical hardware access.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace Hardware Test
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains the target compile-check callback private to this translation unit.
 */
namespace
{

    /**
     * @brief Accepts one target trace record without performing physical I/O.
     *
     * @param[in,out] context
     * Unused nullable callback context.
     *
     * @param[in] level
     * Accepted trace severity.
     *
     * @param[in] message
     * Accepted trace message.
     */
    void acceptOutput(XWalkHal::contextpointer context, XWalkHal::XWalkTraceLevel level, XWalkHal::stringview message)
    {
        static_cast<void>(context);
        static_cast<void>(level);
        static_cast<void>(message);
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Composes and exercises the trace interface for target compilation.
 *
 * @return
 * Zero after the synchronous callback accepts one warning record.
 *
 * @note
 * This compile-check does not access any peripheral or operating-system device.
 */
XWalkHal::int32 main()
{
    XWalkHal::XWalkTrace trace(nullptr, &acceptOutput);
    trace.warning("xWalkTrace target compile check");
    return 0;
}
