/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelTypes.h
 * @brief       Declares language-model roles and backend callback types.
 *
 * @details
 * Defines the provider-neutral operations supplied by an application-owned
 * language-model backend.
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

#ifndef XHAL_RPI5CAR_LANGUAGE_MODEL_TYPES_H
#define XHAL_RPI5CAR_LANGUAGE_MODEL_TYPES_H

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

    /** @brief Identifies the participant responsible for one conversation message. */
    enum class XWalkLanguageModelRole : uint8
    {
        System = 0U,   /**< Supplies persistent model behavior instructions. */
        User = 1U,     /**< Supplies input originating from the application user. */
        Assistant = 2U /**< Supplies a previous language-model response. */
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Callback that replaces the language-model system instructions.
     *
     * @param[in,out] context
     * Nullable non-owning backend context supplied during construction.
     *
     * @param[in] instructions
     * Instruction text valid only for the synchronous callback duration.
     */
    using languagemodelinstructionscallback = void (*)(contextpointer context, stringview instructions);

    /**
     * @brief Callback that replaces the conversation welcome text.
     *
     * @param[in,out] context
     * Nullable non-owning backend context supplied during construction.
     *
     * @param[in] welcome
     * Welcome text valid only for the synchronous callback duration.
     */
    using languagemodelwelcomecallback = void (*)(contextpointer context, stringview welcome);

    /**
     * @brief Callback that configures the retained conversation-message limit.
     *
     * @param[in,out] context
     * Nullable non-owning backend context supplied during construction.
     *
     * @param[in] maximumMessages
     * Non-zero maximum number of conversation messages retained by the backend.
     */
    using languagemodellimitcallback = void (*)(contextpointer context, uint32 maximumMessages);

    /**
     * @brief Callback that adds one message to the backend conversation history.
     *
     * @param[in,out] context
     * Nullable non-owning backend context supplied during construction.
     *
     * @param[in] role
     * Valid system, user, or assistant participant role.
     *
     * @param[in] content
     * Message content valid only for the synchronous callback duration.
     *
     * @param[in] imagePath
     * Optional image path. An empty view indicates that no image is attached.
     */
    using languagemodelmessagecallback = void (*)(contextpointer context,
                                                  XWalkLanguageModelRole role,
                                                  stringview content,
                                                  stringview imagePath);

    /**
     * @brief Callback that submits one prompt and returns the final model response.
     *
     * @param[in,out] context
     * Nullable non-owning backend context supplied during construction.
     *
     * @param[in] promptText
     * Prompt content valid only for the synchronous callback duration.
     *
     * @param[in] imagePath
     * Optional image path. An empty view indicates a text-only request.
     *
     * @return
     * Owned final response text. An empty string represents an empty model response.
     *
     * @warning
     * The callback may block while a local or remote model generates its response.
     */
    using languagemodelpromptcallback = string (*)(contextpointer context, stringview promptText, stringview imagePath);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @brief Groups the complete operations required from a language-model backend.
     *
     * @details
     * Every callback is required and copied during construction. The table owns no
     * backend, network, model, conversation, or image resource.
     */
    struct XWalkLanguageModelCallbacks
    {
            languagemodelinstructionscallback setInstructions{nullptr}; /**< Replaces system instructions. */
            languagemodelwelcomecallback setWelcome{nullptr};           /**< Replaces welcome text. */
            languagemodellimitcallback setMaximumMessages{nullptr};     /**< Sets the retained-message limit. */
            languagemodelmessagecallback addMessage{nullptr};           /**< Adds one conversation message. */
            languagemodelpromptcallback prompt{nullptr};                /**< Generates one final response. */
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_LANGUAGE_MODEL_TYPES_H */
