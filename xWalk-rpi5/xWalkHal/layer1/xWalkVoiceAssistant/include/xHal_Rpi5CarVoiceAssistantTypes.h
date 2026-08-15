/******************************************************************************
 * @file        xHal_Rpi5CarVoiceAssistantTypes.h
 * @brief       Declares voice-assistant configuration and callback types.
 *
 * @details
 * Defines optional lifecycle hooks and owned startup text used by the
 * hardware-independent voice-assistant coordinator.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceAssistant
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

#ifndef XHAL_RPI5CAR_VOICE_ASSISTANT_TYPES_H
#define XHAL_RPI5CAR_VOICE_ASSISTANT_TYPES_H

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
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Callback invoked for a voice-assistant lifecycle event without text.
     *
     * @param[in,out] context
     * Nullable non-owning application context supplied during construction.
     */
    using voiceassistanteventcallback = void (*)(contextpointer context);

    /**
     * @brief Callback invoked for a voice-assistant lifecycle event containing text.
     *
     * @param[in,out] context
     * Nullable non-owning application context supplied during construction.
     *
     * @param[in] text
     * Event text valid only for the synchronous callback duration.
     */
    using voiceassistanttextcallback = void (*)(contextpointer context, stringview text);

    /**
     * @brief Callback that transforms a final model response before speech output.
     *
     * @param[in,out] context
     * Nullable non-owning application context supplied during construction.
     *
     * @param[in] response
     * Final model response valid only for the synchronous callback duration.
     *
     * @return
     * Owned response text to return and optionally synthesize.
     */
    using voiceassistantparsecallback = string (*)(contextpointer context, stringview response);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Contains startup text owned by one voice-assistant coordinator. */
    struct XWalkVoiceAssistantConfiguration
    {
            string instructions{}; /**< System instructions supplied to the language model. */
            string welcome{};      /**< Optional text spoken when the assistant starts. */
    };

    /**
     * @brief Groups optional synchronous voice-assistant lifecycle callbacks.
     *
     * @details
     * Null event callbacks represent the no-op hooks used by the Python base class.
     * A null parse callback preserves the unmodified final model response.
     */
    struct XWalkVoiceAssistantCallbacks
    {
            voiceassistanteventcallback onStart{nullptr};         /**< Reports successful start entry. */
            voiceassistanteventcallback beforeListen{nullptr};    /**< Reports imminent microphone capture. */
            voiceassistanttextcallback afterListen{nullptr};      /**< Reports final recognized text. */
            voiceassistanttextcallback onHeard{nullptr};          /**< Reports non-empty speech used for a round. */
            voiceassistanttextcallback beforeThink{nullptr};      /**< Reports text before model dispatch. */
            voiceassistanttextcallback afterThink{nullptr};       /**< Reports the final model response. */
            voiceassistantparsecallback parseResponse{nullptr};   /**< Optionally transforms the response. */
            voiceassistanttextcallback beforeSay{nullptr};        /**< Reports text before speech output. */
            voiceassistanttextcallback afterSay{nullptr};         /**< Reports completed speech output. */
            voiceassistanteventcallback onRoundComplete{nullptr}; /**< Reports successful round completion. */
            voiceassistanteventcallback onStop{nullptr};          /**< Reports assistant stop completion. */
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_VOICE_ASSISTANT_TYPES_H */
