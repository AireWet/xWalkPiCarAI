/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModel.h
 * @brief       Declares the xWalk provider-neutral language-model coordinator.
 *
 * @details
 * Provides conversation configuration and synchronous prompting through a
 * complete callback table owned and implemented by the application.
 *
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel
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

#ifndef XHAL_RPI5CAR_LANGUAGE_MODEL_H
#define XHAL_RPI5CAR_LANGUAGE_MODEL_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModelTypes.h"

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
     * @class XWalkLanguageModel
     * @brief Coordinates language-model conversation through an injected backend.
     *
     * @details
     * Represents provider-independent behavior without owning credentials, a model,
     * network transport, image encoder, response stream, or conversation storage.
     * The caller-owned backend context must remain valid throughout this object's
     * lifetime. Calls require external serialization.
     */
    class XWalkLanguageModel final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Nullable non-owning language-model backend context.
             *
             * @note
             * Null is permitted only when every callback supports it. Any non-null
             * object must outlive this coordinator and every callback invocation.
             */
            contextpointer backendContextPointer;

            /**
             * @brief Complete backend callback table copied during construction.
             *
             * @note
             * The callbacks are non-owning function pointers and are never replaced.
             */
            XWalkLanguageModelCallbacks callbacks;

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Validates that every required backend callback is non-null.
             *
             * @param[in] backendCallbacks
             * Callback table to validate before storing or invoking it.
             *
             * @throws std::invalid_argument
             * If any callback is null.
             */
            static void validateCallbacks(const XWalkLanguageModelCallbacks& backendCallbacks);

            /**
             * @brief Validates a retained conversation-message limit.
             *
             * @param[in] maximumMessages
             * Requested number of retained messages.
             *
             * @throws std::out_of_range
             * If `maximumMessages` is zero.
             */
            static void validateMaximumMessages(uint32 maximumMessages);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a language-model coordinator from a complete backend table.
             *
             * @param[in,out] context
             * Nullable non-owning backend context. A non-null object must outlive this
             * coordinator, and null requires explicit support from all callbacks.
             *
             * @param[in] backendCallbacks
             * Complete callback table copied into this coordinator.
             *
             * @throws std::invalid_argument
             * If any required callback is null.
             */
            XWalkLanguageModel(contextpointer context, const XWalkLanguageModelCallbacks& backendCallbacks);

            /** @brief Destroys the coordinator without releasing caller-owned backend resources. */
            ~XWalkLanguageModel();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables copying of the non-owning backend binding. */
            XWalkLanguageModel(const XWalkLanguageModel&) = delete;
            /** @brief Disables copy assignment of the non-owning backend binding. */
            XWalkLanguageModel& operator=(const XWalkLanguageModel&) = delete;
            /** @brief Disables moving because backend context identity is retained. */
            XWalkLanguageModel(XWalkLanguageModel&&) = delete;
            /** @brief Disables move assignment because backend context identity is retained. */
            XWalkLanguageModel& operator=(XWalkLanguageModel&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Replaces the system instructions used for later prompts.
             *
             * @param[in] instructions
             * Instruction text forwarded synchronously without encoding changes.
             *
             * @note
             * Empty instructions are preserved. Backend exceptions are propagated.
             */
            void setInstructions(stringview instructions);

            /**
             * @brief Replaces the welcome text associated with the conversation.
             *
             * @param[in] welcome
             * Welcome text forwarded synchronously without encoding changes.
             *
             * @note
             * Empty welcome text is preserved. Backend exceptions are propagated.
             */
            void setWelcome(stringview welcome);

            /**
             * @brief Configures the maximum retained conversation-message count.
             *
             * @param[in] maximumMessages
             * Non-zero number of messages retained according to backend policy.
             *
             * @throws std::out_of_range
             * If `maximumMessages` is zero.
             */
            void setMaximumMessages(uint32 maximumMessages = XHAL_RPI5CAR_LANGUAGE_MODEL_DEFAULT_MAXIMUM_MESSAGES);

            /**
             * @brief Adds one message to the backend conversation history.
             *
             * @param[in] role
             * System, user, or assistant participant responsible for the message.
             *
             * @param[in] content
             * Message content forwarded synchronously without encoding changes.
             *
             * @param[in] imagePath
             * Optional image path. An empty view indicates no attached image.
             *
             * @note
             * The backend owns history truncation and image-processing policy.
             */
            void addMessage(XWalkLanguageModelRole role, stringview content, stringview imagePath = {});

            /**
             * @brief Submits one prompt and returns the final language-model response.
             *
             * @param[in] promptText
             * Prompt text forwarded synchronously without encoding changes.
             *
             * @param[in] imagePath
             * Optional image path. An empty view indicates a text-only prompt.
             *
             * @return
             * Owned final response text, including an empty response when returned by the backend.
             *
             * @warning
             * The injected callback may block on model inference, a process, or a network request.
             */
            string prompt(stringview promptText, stringview imagePath = {});
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_LANGUAGE_MODEL_H */
