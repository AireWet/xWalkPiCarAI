/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordThreadExample.h
 * @brief       Declares the bounded threaded Vosk wake-word example.
 *
 * @details
 * Defines injected worker control, polling, timing, and reporting operations
 * without owning a microphone, recognizer, thread, or platform resource.
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

#ifndef XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_THREAD_EXAMPLE_H
#define XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_THREAD_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Exact wake phrase selected by the upstream example. */
#define XHAL_RPI5CAR_STT_VOSK_WAKE_WORD "hey robot"
/** @brief Highest bounded detection count accepted by the example. */
#define XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_MAXIMUM_DETECTIONS 100U
/** @brief Highest bounded poll count accepted for one detection attempt. */
#define XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_MAXIMUM_POLLS 1'200U

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::example
{

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief Starts one platform wake-word listener. */
    using wakewordstartcallback = void (*)(contextpointer context);
    /** @brief Reports whether the active listener detected the configured phrase. */
    using wakeworddetectedcallback = boolean (*)(contextpointer context);
    /** @brief Stops and joins the active wake-word listener. */
    using wakewordstopcallback = void (*)(contextpointer context);
    /** @brief Waits between source-compatible wake-state polls. */
    using wakewordwaitcallback = void (*)(contextpointer context, uint32 durationMilliseconds);
    /** @brief Reports one literal source-compatible status message. */
    using wakewordreportcallback = void (*)(contextpointer context, stringview message);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Complete injected operation table required by the wake-word example. */
    struct XWalkSttVoskWakeWordThreadExampleCallbacks
    {
            /** @brief Starts one listener worker. */
            wakewordstartcallback startListening{nullptr};
            /** @brief Reads the current wake state. */
            wakeworddetectedcallback isWaked{nullptr};
            /** @brief Stops and joins the current listener worker. */
            wakewordstopcallback stopListening{nullptr};
            /** @brief Waits three seconds between unsuccessful polls. */
            wakewordwaitcallback wait{nullptr};
            /** @brief Reports waiting and detected messages. */
            wakewordreportcallback report{nullptr};
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Coordinates bounded threaded wake-word detection attempts. */
    class XWalkSttVoskWakeWordThreadExample final
    {
        private:
            /** @brief Non-owning context forwarded to every injected operation. */
            contextpointer callbackContext;
            /** @brief Complete validated callback table copied at construction. */
            XWalkSttVoskWakeWordThreadExampleCallbacks callbacks;

        public:
            /**
             * @brief Binds the complete wake-word worker operation table.
             * @param[in,out] context Non-owning context forwarded to every callback.
             * @param[in] exampleCallbacks Table containing five non-null callbacks.
             * @throws std::invalid_argument If any callback is null.
             */
            XWalkSttVoskWakeWordThreadExample(contextpointer context,
                                              const XWalkSttVoskWakeWordThreadExampleCallbacks& exampleCallbacks);

            XWalkSttVoskWakeWordThreadExample(const XWalkSttVoskWakeWordThreadExample&) = delete;
            XWalkSttVoskWakeWordThreadExample(XWalkSttVoskWakeWordThreadExample&&) = delete;
            XWalkSttVoskWakeWordThreadExample& operator=(const XWalkSttVoskWakeWordThreadExample&) = delete;
            XWalkSttVoskWakeWordThreadExample& operator=(XWalkSttVoskWakeWordThreadExample&&) = delete;

            /**
             * @brief Starts, polls, and stops each requested wake-word attempt.
             * @param[in] detectionCount Required successful detection count from one through 100.
             * @param[in] maximumPolls Maximum polls per detection from one through 1,200.
             * @throws std::out_of_range If either bounded count is invalid.
             * @throws std::runtime_error If one attempt reaches its poll limit.
             */
            void run(uint32 detectionCount, uint32 maximumPolls);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_THREAD_EXAMPLE_H */
