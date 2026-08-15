/******************************************************************************
 * @file        xHal_Rpi5CarTraceTypes.h
 * @brief       Declares trace severity and output callback types.
 *
 * @details
 * Defines the typed five-level diagnostic contract ported from the Robot HAT
 * Python basic class without binding the embedded core to a console backend.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
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

#ifndef XHAL_RPI5CAR_TRACE_TYPES_H
#define XHAL_RPI5CAR_TRACE_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

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
     * Enumeration declarations
     ******************************************************************************/

    /**
     * @brief Identifies the severity and filtering priority of one trace record.
     *
     * @details
     * Lower numeric values represent more urgent records. The ordering preserves
     * the Python levels zero through four.
     */
    enum class XWalkTraceLevel : uint8
    {
        Critical = XHAL_RPI5CAR_TRACE_LEVEL_CRITICAL, /**< Critical failure requiring attention. */
        Error = XHAL_RPI5CAR_TRACE_LEVEL_ERROR,       /**< Error that prevents an operation. */
        Warning = XHAL_RPI5CAR_TRACE_LEVEL_WARNING,   /**< Unexpected condition allowing continuation. */
        Info = XHAL_RPI5CAR_TRACE_LEVEL_INFO,         /**< Informational operating-state record. */
        Debug = XHAL_RPI5CAR_TRACE_LEVEL_DEBUG        /**< Detailed diagnostic record. */
    };

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @brief Stores scanner-generated source metadata for one tagged trace.
     *
     * @details
     * The build-time scanner provides the public macro invocation filename and
     * line so runtime output remains identical across GCC and Clang.
     */
    struct XWalkTraceSourceLocation
    {
            /** @brief Project-relative source filename supplied by generated XML. */
            string sourceFile;

            /** @brief One-based line containing the public tagged-trace macro name. */
            uint32 sourceLine{};

            /** @brief Preserved callback and record priority from zero through three. */
            uint8 priority{};
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Callback invoked synchronously for one accepted trace record.
     *
     * @param[in,out] context
     * Non-owning application context supplied to `XWalkTrace`; null is permitted
     * when the callback does not require state.
     *
     * @param[in] level
     * Severity assigned to the record.
     *
     * @param[in] message
     * Non-owning text view valid only for the duration of the callback.
     *
     * @warning
     * The callback must not retain `message` beyond the invocation.
     */
    using traceoutputcallback = void (*)(contextpointer context, XWalkTraceLevel level, stringview message);

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_TRACE_TYPES_H */
