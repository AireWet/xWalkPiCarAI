/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskStreamExample.h
 * @brief       Declares the bounded streaming Vosk speech example.
 *
 * @details
 * Defines injected listening and reporting operations that preserve the
 * upstream prompt plus partial/final result flow without owning hardware.
 *
 * @project     xWalk Firmware
 * @module      xExample
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_STT_VOSK_STREAM_EXAMPLE_H
#define XHAL_RPI5CAR_STT_VOSK_STREAM_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Highest bounded recognition-session count accepted by the example. */
#define XHAL_RPI5CAR_STT_VOSK_STREAM_EXAMPLE_MAXIMUM_SESSIONS 100U

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::example
{

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief Reports one partial or final recognition result. */
    using sttvoskstreamresultcallback = void (*)(contextpointer context, boolean done, stringview text);

    /** @brief Runs one bounded recognition stream and reports each yielded result. */
    using sttvoskstreamlistencallback = void (*)(contextpointer context,
                                                 uint32 timeoutMs,
                                                 sttvoskstreamresultcallback reportResult);

    /** @brief Reports the source-compatible speech prompt. */
    using sttvoskstreampromptcallback = void (*)(contextpointer context);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Complete operation table required by the streaming Vosk example. */
    struct XWalkSttVoskStreamExampleCallbacks
    {
            /** @brief Runs one bounded recognition stream. */
            sttvoskstreamlistencallback listen{nullptr};
            /** @brief Reports `Say something` before each stream. */
            sttvoskstreampromptcallback reportPrompt{nullptr};
            /** @brief Reports each partial or final result. */
            sttvoskstreamresultcallback reportResult{nullptr};
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Coordinates bounded Vosk streaming sessions through injected operations. */
    class XWalkSttVoskStreamExample final
    {
        private:
            /** @brief Non-owning context forwarded to every operation. */
            contextpointer callbackContext;
            /** @brief Complete validated operation table copied at construction. */
            XWalkSttVoskStreamExampleCallbacks callbacks;

        public:
            /**
             * @brief Binds the complete streaming speech operation table.
             * @param[in,out] context Non-owning context forwarded to every callback.
             * @param[in] exampleCallbacks Table containing three non-null callbacks.
             * @throws std::invalid_argument If any callback is null.
             */
            XWalkSttVoskStreamExample(contextpointer context,
                                      const XWalkSttVoskStreamExampleCallbacks& exampleCallbacks);

            /** @brief Prevents copying of non-owning callback bindings. */
            XWalkSttVoskStreamExample(const XWalkSttVoskStreamExample&) = delete;
            /** @brief Prevents moving of non-owning callback bindings. */
            XWalkSttVoskStreamExample(XWalkSttVoskStreamExample&&) = delete;
            /** @brief Prevents copy assignment of non-owning callback bindings. */
            XWalkSttVoskStreamExample& operator=(const XWalkSttVoskStreamExample&) = delete;
            /** @brief Prevents move assignment of non-owning callback bindings. */
            XWalkSttVoskStreamExample& operator=(XWalkSttVoskStreamExample&&) = delete;

            /**
             * @brief Runs the requested number of bounded recognition sessions.
             * @param[in] sessionCount Session count from one through 100.
             * @param[in] timeoutMs Per-session timeout from one through 300,000 milliseconds.
             * @throws std::out_of_range If either bounded argument is invalid.
             */
            void run(uint32 sessionCount, uint32 timeoutMs);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_STT_VOSK_STREAM_EXAMPLE_H */
