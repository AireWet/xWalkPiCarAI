/******************************************************************************
 * @file        xHal_Rpi5CarTrace.h
 * @brief       Declares the xWalk diagnostic trace interface.
 *
 * @details
 * Provides validated severity selection, threshold filtering, and synchronous
 * delivery through a caller-provided output callback.
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

#ifndef XHAL_RPI5CAR_TRACE_H
#define XHAL_RPI5CAR_TRACE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTraceTypes.h"

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
 * @class XWalkTrace
 * @brief Filters and forwards diagnostic records to an injected output callback.
 *
 * @details
 * Preserves the critical-through-debug severity behavior of `_Basic_class`
 * while leaving timestamps, formatting, and physical output to the application.
 * The callback and its optional non-owning context must remain usable throughout
 * every trace call. Calls and level changes require external synchronization.
 */
class XWalkTrace final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Non-owning application context passed to the output callback.
         *
         * @note
         * Null is permitted when the callback does not require state. Any
         * non-null object must outlive this trace object and all trace calls.
         */
        contextpointer outputContextPointer;

        /**
         * @brief Non-null synchronous output function supplied during construction.
         *
         * @note
         * The callback is never owned or modified by this object.
         */
        traceoutputcallback outputCallback;

        /** @brief Highest numeric severity currently accepted by the filter. */
        XWalkTraceLevel levelValue;

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Converts and validates one numeric Python-compatible level.
         *
         * @param[in] level
         * Severity number in the inclusive range zero through four.
         *
         * @return
         * Corresponding typed trace level.
         *
         * @throws std::out_of_range
         * If `level` is greater than four.
         */
        static XWalkTraceLevel parseLevel(uint8 level);

        /**
         * @brief Converts and validates one lowercase Python-compatible level name.
         *
         * @param[in] levelName
         * One of `critical`, `error`, `warning`, `info`, or `debug`.
         *
         * @return
         * Corresponding typed trace level.
         *
         * @throws std::invalid_argument
         * If `levelName` is not supported.
         */
        static XWalkTraceLevel parseLevel(stringview levelName);

        /**
         * @brief Validates a typed trace level.
         *
         * @param[in] level
         * Trace severity to validate.
         *
         * @return
         * The validated level.
         *
         * @throws std::out_of_range
         * If `level` does not identify a supported severity.
         */
        static XWalkTraceLevel validateLevel(XWalkTraceLevel level);

        /**
         * @brief Reports whether a severity passes the configured threshold.
         *
         * @param[in] level
         * Valid severity being considered.
         *
         * @return
         * `true` when the record must be forwarded; otherwise `false`.
         */
        boolean accepts(XWalkTraceLevel level) const noexcept;

        /**
         * @brief Forwards one record when its severity passes the threshold.
         *
         * @param[in] level
         * Valid severity assigned to the record.
         *
         * @param[in] message
         * Message view that remains valid throughout the synchronous callback.
         *
         * @note
         * Any exception raised by the injected callback is propagated.
         */
        void write(XWalkTraceLevel level, stringview message) const;

        /**
         * @brief Reports a level change through the debug channel when enabled.
         *
         * @post
         * A debug record is delivered only when the new threshold accepts debug.
         */
        void reportLevelChange() const;

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a trace interface with a typed severity threshold.
         *
         * @param[in,out] outputContext
         * Non-owning callback context; null is permitted for stateless callbacks.
         *
         * @param[in] output
         * Non-null synchronous record-output callback.
         *
         * @param[in] level
         * Initial severity threshold; warning is used by default.
         *
         * @post
         * Records at or above the configured urgency are forwarded synchronously.
         *
         * @throws std::invalid_argument
         * If `output` is null.
         *
         * @throws std::out_of_range
         * If `level` does not identify a supported severity.
         */
        explicit XWalkTrace(contextpointer outputContext, traceoutputcallback output,
            XWalkTraceLevel level = XWalkTraceLevel::Warning);

        /**
         * @brief Constructs a trace interface from a numeric severity threshold.
         *
         * @param[in,out] outputContext
         * Non-owning callback context; null is permitted for stateless callbacks.
         *
         * @param[in] output
         * Non-null synchronous record-output callback.
         *
         * @param[in] level
         * Initial Python-compatible severity number from zero through four.
         *
         * @throws std::invalid_argument
         * If `output` is null.
         *
         * @throws std::out_of_range
         * If `level` is greater than four.
         */
        XWalkTrace(contextpointer outputContext, traceoutputcallback output, uint8 level);

        /**
         * @brief Constructs a trace interface from a textual severity threshold.
         *
         * @param[in,out] outputContext
         * Non-owning callback context; null is permitted for stateless callbacks.
         *
         * @param[in] output
         * Non-null synchronous record-output callback.
         *
         * @param[in] levelName
         * Initial lowercase Python-compatible severity name.
         *
         * @throws std::invalid_argument
         * If `output` is null or `levelName` is unsupported.
         */
        XWalkTrace(contextpointer outputContext, traceoutputcallback output, stringview levelName);

        /** @brief Destroys the trace interface without releasing its non-owning context. */
        ~XWalkTrace();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        XWalkTrace(const XWalkTrace&) = delete;
        XWalkTrace& operator=(const XWalkTrace&) = delete;
        XWalkTrace(XWalkTrace&&) = delete;
        XWalkTrace& operator=(XWalkTrace&&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Selects a typed severity threshold.
         *
         * @param[in] level
         * Highest numeric severity accepted by the filter.
         *
         * @post
         * `level()` equals `level`.
         *
         * @throws std::out_of_range
         * If `level` does not identify a supported severity.
         */
        void setLevel(XWalkTraceLevel level);

        /**
         * @brief Selects a numeric Python-compatible severity threshold.
         *
         * @param[in] level
         * Severity number in the inclusive range zero through four.
         *
         * @post
         * `level()` equals the corresponding typed level.
         *
         * @throws std::out_of_range
         * If `level` is greater than four.
         */
        void setLevel(uint8 level);

        /**
         * @brief Selects a textual Python-compatible severity threshold.
         *
         * @param[in] levelName
         * One of `critical`, `error`, `warning`, `info`, or `debug`.
         *
         * @post
         * `level()` equals the corresponding typed level.
         *
         * @throws std::invalid_argument
         * If `levelName` is unsupported.
         */
        void setLevel(stringview levelName);

        /**
         * @brief Returns the configured typed severity threshold.
         *
         * @return
         * Highest numeric severity currently accepted by the filter.
         */
        XWalkTraceLevel level() const noexcept;

        /**
         * @brief Returns the lowercase name of the configured threshold.
         *
         * @return
         * Static non-owning view containing the Python-compatible level name.
         */
        stringview levelName() const noexcept;

        /**
         * @brief Emits a critical record when enabled.
         *
         * @param[in] message
         * Non-owning record text consumed synchronously by the callback.
         */
        void critical(stringview message) const;

        /**
         * @brief Emits an error record when enabled.
         *
         * @param[in] message
         * Non-owning record text consumed synchronously by the callback.
         */
        void error(stringview message) const;

        /**
         * @brief Emits a warning record when enabled.
         *
         * @param[in] message
         * Non-owning record text consumed synchronously by the callback.
         */
        void warning(stringview message) const;

        /**
         * @brief Emits an informational record when enabled.
         *
         * @param[in] message
         * Non-owning record text consumed synchronously by the callback.
         */
        void info(stringview message) const;

        /**
         * @brief Emits a debug record when enabled.
         *
         * @param[in] message
         * Non-owning record text consumed synchronously by the callback.
         */
        void debug(stringview message) const;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_TRACE_H */
