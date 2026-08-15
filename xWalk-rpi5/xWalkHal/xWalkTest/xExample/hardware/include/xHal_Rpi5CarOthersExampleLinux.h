/******************************************************************************
 * @file        xHal_Rpi5CarOthersExampleLinux.h
 * @brief       Declares generic OpenAI-compatible HTTPS example composition.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
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

#ifndef XHAL_RPI5CAR_OTHERS_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_OTHERS_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarOthersExample.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains contracts and adapters for ported example programs.
 */
namespace xwalk::hal::example
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Composes generic provider chat with libcurl and terminal I/O. */
    class XWalkOthersExampleLinux final
    {
        protected:
            /** @brief Prints `>>> ` and reads one terminal input line. */
            static boolean readPrompt(contextpointer context, string& inputText);
            /** @brief Writes one welcome or response fragment to standard output. */
            static void write(contextpointer context, stringview text, boolean appendNewline, boolean flushOutput);

        public:
            /**
             * @brief Runs bounded chat through a selected OpenAI-compatible provider.
             * @param[in] apiKey Non-empty credential sourced outside process arguments.
             * @param[in] endpoint Complete non-empty chat-completions endpoint.
             * @param[in] model Non-empty provider model identifier.
             * @param[in] maximumPrompts Prompt limit from one through 100.
             * @warning Sends user prompts to a remote service over HTTPS.
             */
            void run(stringview apiKey, stringview endpoint, stringview model, uint32 maximumPrompts);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_OTHERS_EXAMPLE_LINUX_H */
