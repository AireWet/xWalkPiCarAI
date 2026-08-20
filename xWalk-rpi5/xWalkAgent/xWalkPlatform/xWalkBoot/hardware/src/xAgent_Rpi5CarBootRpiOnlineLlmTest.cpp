/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiOnlineLlmTest.cpp
 * @brief       Composes the Raspberry Pi online language-model test mode.
 *
 * @details
 * Reads the configured credential environment name and publishes one
 * authenticated OpenAI-compatible language-model service.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"

#include "xHal_Rpi5CarTrace.h"
#include <cstdlib>

namespace xwalk::agent
{

    /**
     * @brief Runs the configured online language-model test.
     * @param[in] parameters Non-owning application callback and configuration
     * dependency valid through this synchronous composition.
     * @return Status returned by the configured callback.
     * @throws std::runtime_error If the configured credential is absent.
     * @pre `parameters.callback` and `parameters.config` are non-null.
     */
    agent::int32 XWalkBootRpi::runOnlineLlmTest(const xAgentContext& parameters)
    {
        hal::XWalkConfigStore& config = *parameters.config;
        const agent::string apiKeyEnvironment = config.get("online_llm_api_key_environment", "OPENAI_API_KEY");
        const agent::cstring apiKey = std::getenv(apiKeyEnvironment.c_str());
        const agent::boolean apiKeyMissing = static_cast<agent::boolean>((apiKey == nullptr) || (apiKey[0U] == '\0'));
        if (apiKeyMissing)
        {
            const std::string exceptionMessage =
                std::string(apiKeyEnvironment).append(" must be set for online-llm-test");
            XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, exceptionMessage);
        }
        hal::XWalkLanguageModelOllama modelBackend(
            hal::XWalkLanguageModelHttpDialect::OpenAiChatCompletions,
            config.get("online_llm_endpoint", "https://api.openai.com/v1/chat/completions"),
            config.get("online_llm_model", "gpt-4o"),
            apiKey);
        hal::XWalkLanguageModel languageModel(&modelBackend, modelBackend.callbacks());
        XWalkBootServices services{};
        services.languageModel = &languageModel;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
