/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWithoutStreamExample.h
 * @brief       Declares the bounded non-streaming Vosk speech example.
 *
 * @details
 * Defines injected synchronous listening and reporting operations that retain
 * the upstream prompt and unlabeled final-transcript output.
 *
 * @project     xWalk Firmware
 * @module      xExample
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_STT_VOSK_WITHOUT_STREAM_EXAMPLE_H
#define XHAL_RPI5CAR_STT_VOSK_WITHOUT_STREAM_EXAMPLE_H

#include "xHal_Rpi5CarCommon.h"

/** @brief Highest bounded recognition-session count accepted by the example. */
#define XHAL_RPI5CAR_STT_VOSK_WITHOUT_STREAM_EXAMPLE_MAXIMUM_SESSIONS 100U

namespace xwalk::hal::example
{

    /** @brief Performs one bounded synchronous recognition request. */
    using sttvoskwithoutstreamlistencallback = string (*)(contextpointer context, uint32 timeoutMs);
    /** @brief Reports one literal prompt or final transcript. */
    using sttvoskwithoutstreamreportcallback = void (*)(contextpointer context, stringview text);

    /** @brief Complete operation table required by the non-streaming example. */
    struct XWalkSttVoskWithoutStreamExampleCallbacks
    {
            /** @brief Returns one final recognition result. */
            sttvoskwithoutstreamlistencallback listen{nullptr};
            /** @brief Reports prompts and final results. */
            sttvoskwithoutstreamreportcallback report{nullptr};
    };

    /** @brief Coordinates bounded non-streaming Vosk recognition sessions. */
    class XWalkSttVoskWithoutStreamExample final
    {
        private:
            /** @brief Non-owning context forwarded to every operation. */
            contextpointer callbackContext;
            /** @brief Complete validated callback table copied at construction. */
            XWalkSttVoskWithoutStreamExampleCallbacks callbacks;

        public:
            /**
             * @brief Binds the complete non-streaming speech operation table.
             * @param[in,out] context Non-owning context forwarded to every callback.
             * @param[in] exampleCallbacks Table containing two non-null callbacks.
             * @throws std::invalid_argument If either callback is null.
             */
            XWalkSttVoskWithoutStreamExample(contextpointer context,
                                             const XWalkSttVoskWithoutStreamExampleCallbacks& exampleCallbacks);

            XWalkSttVoskWithoutStreamExample(const XWalkSttVoskWithoutStreamExample&) = delete;
            XWalkSttVoskWithoutStreamExample(XWalkSttVoskWithoutStreamExample&&) = delete;
            XWalkSttVoskWithoutStreamExample& operator=(const XWalkSttVoskWithoutStreamExample&) = delete;
            XWalkSttVoskWithoutStreamExample& operator=(XWalkSttVoskWithoutStreamExample&&) = delete;

            /**
             * @brief Prompts, listens, and reports one result for every session.
             * @param[in] sessionCount Session count from one through 100.
             * @param[in] timeoutMs Per-session timeout from one through 300,000 milliseconds.
             * @throws std::out_of_range If either bounded argument is invalid.
             */
            void run(uint32 sessionCount, uint32 timeoutMs);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_STT_VOSK_WITHOUT_STREAM_EXAMPLE_H */
