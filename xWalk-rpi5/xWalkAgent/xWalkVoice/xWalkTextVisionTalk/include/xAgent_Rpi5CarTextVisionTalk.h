/******************************************************************************
 * @file        xAgent_Rpi5CarTextVisionTalk.h
 * @brief       Declares the example-17 image-grounded text conversation Agent.
 * @project     xWalk Firmware
 * @module      xWalkTextVisionTalk
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_TEXT_VISION_TALK_H
#define XAGENT_RPI5CAR_TEXT_VISION_TALK_H

#include "xAgent_Rpi5CarCameraCapture.h"
#include "xAgent_Rpi5CarTextVisionTalkTypes.h"
#include "xHal_Rpi5CarLanguageModel.h"

/** @namespace xwalk::agent @brief Contains application coordinators for xWalk firmware. */
namespace xwalk::agent
{

    /** @brief Coordinates typed prompts with a newly captured image and language model. */
    class XWalkTextVisionTalk final
    {
        private:
            /** @brief Non-owning model pointer whose caller-owned object must outlive this Agent. */
            hal::XWalkLanguageModel* languageModelObject{nullptr};
            /** @brief Non-owning capture pointer whose caller-owned object must outlive this Agent. */
            XWalkCameraCapture* cameraCaptureObject{nullptr};
            /** @brief Nullable non-owning context forwarded synchronously to callbacks. */
            agent::contextpointer callbackContext{nullptr};
            /** @brief Complete synchronous callback table copied during construction. */
            XWalkTextVisionTalkCallbacks callbacks{};
            /** @brief Owned and validated source-compatible conversation settings. */
            XWalkTextVisionTalkConfiguration configuration{};

        protected:
            /** @brief Trims and lowercases exit-control text. */
            static agent::string normalize(agent::stringview text);
            /** @brief Validates the complete callback table and bounded settings. */
            static void validate(const XWalkTextVisionTalkCallbacks& backendCallbacks,
                                 const XWalkTextVisionTalkConfiguration& talkConfiguration);

        public:
            /**
             * @brief Binds caller-owned model, camera, context, and callbacks.
             * @param[in,out] languageModel Model coordinator that must outlive this Agent.
             * @param[in,out] cameraCapture Capture Agent that must outlive this Agent.
             * @param[in,out] context Nullable callback context that must outlive callback use.
             * @param[in] backendCallbacks Complete synchronous callback table.
             * @param[in] talkConfiguration Owned conversation and warm-up settings.
             */
            XWalkTextVisionTalk(hal::XWalkLanguageModel& languageModel,
                                XWalkCameraCapture& cameraCapture,
                                agent::contextpointer context,
                                const XWalkTextVisionTalkCallbacks& backendCallbacks,
                                const XWalkTextVisionTalkConfiguration& talkConfiguration = {});
            /** @brief Releases no caller-owned model, camera, context, or callback. */
            ~XWalkTextVisionTalk() = default;

            XWalkTextVisionTalk(const XWalkTextVisionTalk&) = delete;
            XWalkTextVisionTalk& operator=(const XWalkTextVisionTalk&) = delete;
            XWalkTextVisionTalk(XWalkTextVisionTalk&&) = delete;
            XWalkTextVisionTalk& operator=(XWalkTextVisionTalk&&) = delete;

            /** @brief Runs typed image-grounded prompts until exit, quit, or cancellation. */
            agent::int32 run();
            /** @brief Requires no shutdown because model and capture calls are synchronous. */
            void stop() noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_TEXT_VISION_TALK_H */
