/******************************************************************************
 * @file        xAgent_Rpi5CarOnlineLlmTestLifecycle.cpp
 * @brief       Implements online-LLM-test construction and validation.
 * @project     xWalk Firmware
 * @module      xWalkOnlineLlmTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarOnlineLlmTest.h"

#include "xHal_Rpi5CarTrace.h"
/** @namespace xwalk::agent @brief Contains application coordinators for xWalk
 * firmware. */
namespace xwalk::agent
{

    /**
     * @brief Binds caller-owned model, callbacks, and conversation settings.
     * @param[in,out] languageModel Model coordinator that must outlive this Agent.
     * @param[in,out] context Nullable context that must outlive callback use.
     * @param[in] backendCallbacks Complete synchronous callback table.
     * @param[in] testConfiguration Owned source-compatible settings.
     */
    XWalkOnlineLlmTest::XWalkOnlineLlmTest(hal::XWalkLanguageModel& languageModel,
                                           agent::contextpointer context,
                                           const XWalkOnlineLlmTestCallbacks& backendCallbacks,
                                           const XWalkOnlineLlmTestConfiguration& testConfiguration)
        : languageModelObject(&languageModel), callbackContext(context), callbacks(backendCallbacks),
          configuration(testConfiguration)
    {
        validate(callbacks, configuration);
    }

    /**
     * @brief Validates callbacks and bounded source configuration.
     * @param[in] backendCallbacks Callback table requiring three non-null
     * functions.
     * @param[in] testConfiguration Settings requiring a prompt and non-zero
     * history.
     * @throws std::invalid_argument If a callback or prompt is missing.
     * @throws std::out_of_range If the retained-message limit is zero.
     */
    void XWalkOnlineLlmTest::validate(const XWalkOnlineLlmTestCallbacks& backendCallbacks,
                                      const XWalkOnlineLlmTestConfiguration& testConfiguration)
    {
        if ((backendCallbacks.output == nullptr) || (backendCallbacks.input == nullptr) ||
            (backendCallbacks.shouldContinue == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Online-LLM-test callbacks must be complete");
        }
        const agent::boolean promptTextEmpty = static_cast<agent::boolean>(testConfiguration.promptText.empty());
        if (promptTextEmpty)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Online-LLM-test prompt must not be empty");
        }
        if (testConfiguration.maximumMessages == 0U)
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "Online-LLM-test message limit must not be zero");
        }
    }

} /* namespace xwalk::agent */
