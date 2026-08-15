/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordExample.h
 * @brief       Declares the bounded synchronous Vosk wake-word example.
 *
 * @details
 * Defines injected recognition and reporting operations while preserving the
 * upstream prompt, wake phrase, synchronous wait, and detection message.
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

#ifndef XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_H
#define XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Exact wake phrase configured by the upstream example. */
#define XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_PHRASE "hey robot"
/** @brief Highest bounded recognition-attempt count accepted by the example. */
#define XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_MAXIMUM_ATTEMPTS 1'200U

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::example
{

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief Performs one bounded synchronous recognition attempt. */
    using wakewordexamplelistencallback = string (*)(contextpointer context, uint32 timeoutMs);
    /** @brief Reports one literal source-compatible status line. */
    using wakewordexamplereportcallback = void (*)(contextpointer context, stringview message);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Complete injected operation table required by the wake-word example. */
    struct XWalkSttVoskWakeWordExampleCallbacks
    {
            /** @brief Returns one final bounded recognition result. */
            wakewordexamplelistencallback listen{nullptr};
            /** @brief Reports the prompt and successful detection message. */
            wakewordexamplereportcallback report{nullptr};
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Waits synchronously for the configured wake phrase through callbacks. */
    class XWalkSttVoskWakeWordExample final
    {
        private:
            /** @brief Non-owning context forwarded to every injected operation. */
            contextpointer callbackContext;
            /** @brief Complete validated operation table copied at construction. */
            XWalkSttVoskWakeWordExampleCallbacks callbacks;

        protected:
            /** @brief Tests one transcript for the case-insensitive wake phrase. */
            static boolean containsWakeWord(stringview transcript);

        public:
            /**
             * @brief Binds the complete synchronous wake-word operation table.
             * @param[in,out] context Non-owning context forwarded to every callback.
             * @param[in] exampleCallbacks Table containing two non-null callbacks.
             * @throws std::invalid_argument If either callback is null.
             */
            XWalkSttVoskWakeWordExample(contextpointer context,
                                        const XWalkSttVoskWakeWordExampleCallbacks& exampleCallbacks);

            XWalkSttVoskWakeWordExample(const XWalkSttVoskWakeWordExample&) = delete;
            XWalkSttVoskWakeWordExample(XWalkSttVoskWakeWordExample&&) = delete;
            XWalkSttVoskWakeWordExample& operator=(const XWalkSttVoskWakeWordExample&) = delete;
            XWalkSttVoskWakeWordExample& operator=(XWalkSttVoskWakeWordExample&&) = delete;

            /**
             * @brief Waits synchronously until recognition finds `hey robot`.
             * @param[in] maximumAttempts Attempt limit from one through 1,200.
             * @param[in] timeoutMs Per-attempt limit from one through 300,000 milliseconds.
             * @throws std::out_of_range If either bound is invalid.
             * @throws std::runtime_error If no attempt contains the wake phrase.
             */
            void run(uint32 maximumAttempts, uint32 timeoutMs);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_H */
