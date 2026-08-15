/******************************************************************************
 * @file        xAgent_Rpi5CarOnlineLlmTest.h
 * @brief       Declares the example-18 online text conversation Agent.
 * @project     xWalk Firmware
 * @module      xWalkOnlineLlmTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_ONLINE_LLM_TEST_H
#define XAGENT_RPI5CAR_ONLINE_LLM_TEST_H

#include "xAgent_Rpi5CarOnlineLlmTestTypes.h"
#include "xHal_Rpi5CarLanguageModel.h"

/** @namespace xwalk::agent @brief Contains application coordinators for xWalk firmware. */
namespace xwalk::agent
{

    /** @brief Coordinates typed prompts with one caller-owned online language model. */
    class XWalkOnlineLlmTest final
    {
        private:
            /** @brief Non-owning model pointer whose caller-owned object must outlive this Agent. */
            hal::XWalkLanguageModel* languageModelObject{nullptr};
            /** @brief Nullable non-owning context forwarded synchronously to callbacks. */
            agent::contextpointer callbackContext{nullptr};
            /** @brief Complete synchronous callback table copied during construction. */
            XWalkOnlineLlmTestCallbacks callbacks{};
            /** @brief Owned and validated source-compatible conversation settings. */
            XWalkOnlineLlmTestConfiguration configuration{};

        protected:
            /** @brief Validates the complete callback table and bounded settings. */
            static void validate(const XWalkOnlineLlmTestCallbacks& backendCallbacks,
                                 const XWalkOnlineLlmTestConfiguration& testConfiguration);

        public:
            /**
             * @brief Binds a caller-owned model, callback context, and configuration.
             * @param[in,out] languageModel Model coordinator that must outlive this Agent.
             * @param[in,out] context Nullable context that must outlive callback use.
             * @param[in] backendCallbacks Complete synchronous callback table.
             * @param[in] testConfiguration Owned conversation settings.
             */
            XWalkOnlineLlmTest(hal::XWalkLanguageModel& languageModel,
                               agent::contextpointer context,
                               const XWalkOnlineLlmTestCallbacks& backendCallbacks,
                               const XWalkOnlineLlmTestConfiguration& testConfiguration = {});
            /** @brief Releases no caller-owned model, context, or callback. */
            ~XWalkOnlineLlmTest() = default;

            XWalkOnlineLlmTest(const XWalkOnlineLlmTest&) = delete;
            XWalkOnlineLlmTest& operator=(const XWalkOnlineLlmTest&) = delete;
            XWalkOnlineLlmTest(XWalkOnlineLlmTest&&) = delete;
            XWalkOnlineLlmTest& operator=(XWalkOnlineLlmTest&&) = delete;

            /** @brief Runs typed text-only prompts until foreground cancellation. */
            agent::int32 run();
            /** @brief Requires no shutdown because model calls are synchronous. */
            void stop() noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_ONLINE_LLM_TEST_H */
