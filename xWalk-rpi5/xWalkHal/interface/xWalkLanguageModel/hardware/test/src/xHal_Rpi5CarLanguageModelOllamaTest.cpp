/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelOllamaTest.cpp
 * @brief       Verifies HTTP language-model provider behavior without network
 *access.
 *
 * @details
 * Covers JSON conversion, roles, history truncation, image encoding, Unicode
 * response decoding, transport failures, malformed responses, and all bounds.
 *
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Ollama Host Test
 *
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModelOllama.h"
#include "xHal_Rpi5CarLanguageModel.h"
#include "xHal_Rpi5CarLanguageModelSimulationConfig.h"

#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"

#include <fstream>
#include "xHal_Rpi5CarLanguageModelOllamaTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TestTransport = ::xwalk::source_types::xhal_rpi5carlanguagemodelollamatest::TestTransport;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains deterministic HTTP state and test scenarios.
 */
namespace
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /******************************************************************************
     * Private function definitions
     ******************************************************************************/

    /**
     * @brief Records one bounded request and returns a configured JSON response.
     *
     * @param[in,out] context Non-null test transport context.
     * @param[in] endpoint Ollama chat endpoint retained for assertions.
     * @param[in] requestJson Complete request retained for assertions.
     * @param[in] authorizationHeader Empty or authenticated header retained for
     * assertions.
     * @param[in] timeoutMs Request timeout retained for assertions.
     * @param[in] maximumResponseBytes Response bound retained for assertions.
     * @return Configured owned JSON response.
     * @throws std::runtime_error When transport failure is enabled.
     */
    XWalkHal::string postJson(XWalkHal::contextpointer context,
                              XWalkHal::stringview endpoint,
                              XWalkHal::stringview requestJson,
                              XWalkHal::stringview authorizationHeader,
                              XWalkHal::uint32 timeoutMs,
                              XWalkHal::size maximumResponseBytes)
    {
        TestTransport& transport = *static_cast<TestTransport*>(context);
        ++transport.requestCount;
        transport.endpoint = endpoint;
        transport.request = requestJson;
        transport.authorizationHeader = authorizationHeader;
        transport.timeoutMs = timeoutMs;
        transport.maximumResponseBytes = maximumResponseBytes;
        if (transport.fail)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Simulated Ollama transport failure");
        }
        return transport.response;
    }

    /**
     * @brief Returns the complete deterministic HTTP operation table.
     * @return One non-null fake JSON POST callback.
     */
    XWalkHal::XWalkLanguageModelOllamaOperations operations()
    {
        return {&postJson};
    }

    /**
     * @brief Writes a three-byte image fixture below the supplied test directory.
     *
     * @param[in] testDirectory Existing test-owned output directory.
     * @return Path of the completed fixture.
     */
    XWalkHal::filesystempath writeImageFixture(const XWalkHal::filesystempath& testDirectory)
    {
        const XWalkHal::filesystempath imagePath = testDirectory / "ollama-image.bin";
        XWalkHal::outputfilestream image(imagePath, XWalkHal::FILE_OPEN_WRITE_TRUNCATE);
        const XWalkHal::string bytes{"\x01\x02\x03", 3U};
        image << bytes;
        xwalk::hal::test::requireTestCondition(image.good());
        return imagePath;
    }

    /**
     * @brief Verifies complete request conversion and Unicode response decoding.
     *
     * @param[in] imagePath Existing three-byte test image.
     */
    void testRequestAndResponse(const XWalkHal::filesystempath& imagePath)
    {
        TestTransport transport;
        transport.response = "{\"model\":\"tiny\",\"message\":{\"role\":\"assistant\","
                             "\"content\":\"hello\\n\\u00E5\\uD83D\\uDE80\"},\"done\":true}";
        XWalkHal::XWalkLanguageModelOllama backend(
            &transport, operations(), "http://127.0.0.1:11434/api/chat", "tiny", 5'000U);
        XWalkHal::XWalkLanguageModel model(&backend, backend.callbacks());
        model.setInstructions("answer \"briefly\"");
        model.setWelcome("welcome");
        model.setMaximumMessages(4U);
        model.addMessage(XWalkHal::XWalkLanguageModelRole::Assistant, "prior");
        const XWalkHal::string result = model.prompt("inspect\nimage", imagePath.string());
        xwalk::hal::test::requireTestCondition(result == u8"hello\nå🚀");
        xwalk::hal::test::requireTestCondition(transport.endpoint == "http://127.0.0.1:11434/api/chat");
        xwalk::hal::test::requireTestCondition(transport.timeoutMs == 5'000U);
        xwalk::hal::test::requireTestCondition(transport.maximumResponseBytes ==
                                               XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_RESPONSE_BYTES);
        xwalk::hal::test::requireTestCondition(transport.request.find("\"model\":\"tiny\"") != XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("answer \\\"briefly\\\"") !=
                                               XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("inspect\\u000Aimage") != XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("\"images\":[\"AQID\"]") !=
                                               XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("\"stream\":false") != XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.authorizationHeader.empty());
    }

    /**
     * @brief Verifies authenticated OpenAI-compatible request and response
     * conversion.
     * @param[in] imagePath Existing three-byte test image.
     */
    void testOpenAiCompatibleRequest(const XWalkHal::filesystempath& imagePath)
    {
        TestTransport transport;
        transport.response = "{\"choices\":[{\"index\":0,\"message\":{\"role\":\"assistant\","
                             "\"content\":\"cloud-ready\"}}]}";
        XWalkHal::XWalkLanguageModelHttp backend(&transport,
                                                 operations(),
                                                 XWalkHal::XWalkLanguageModelHttpDialect::OpenAiChatCompletions,
                                                 "https://api.example/v1/chat/completions",
                                                 "selected-model",
                                                 "test-secret",
                                                 5'000U,
                                                 321U);
        XWalkHal::XWalkLanguageModel model(&backend, backend.callbacks());
        model.setInstructions("be concise");
        const XWalkHal::string result = model.prompt("inspect", imagePath.string());
        xwalk::hal::test::requireTestCondition(result == "cloud-ready");
        xwalk::hal::test::requireTestCondition(transport.authorizationHeader == "Authorization: Bearer test-secret");
        xwalk::hal::test::requireTestCondition(transport.request.find("test-secret") == XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("\"model\":\"selected-model\"") !=
                                               XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("\"max_tokens\":321") != XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("\"type\":\"image_url\"") !=
                                               XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("data:image/jpeg;base64,AQID") !=
                                               XWalkHal::string::npos);
    }

    /** @brief Verifies deployment provider-name mapping and unsupported Kiro
     * rejection. */
    void testProviderNames()
    {
        xwalk::hal::test::requireTestCondition(XWalkHal::XWalkLanguageModelHttp::dialectFromString("ollama") ==
                                               XWalkHal::XWalkLanguageModelHttpDialect::Ollama);
        for (const XWalkHal::stringview provider :
             {"openai", "chatgpt", "gemini", "grok", "xai", "claude", "anthropic", "openai_compatible"})
        {
            xwalk::hal::test::requireTestCondition(XWalkHal::XWalkLanguageModelHttp::dialectFromString(provider) ==
                                                   XWalkHal::XWalkLanguageModelHttpDialect::OpenAiChatCompletions);
        }
        xwalk::hal::test::expectFailure(
            []()
            {
                static_cast<void>(XWalkHal::XWalkLanguageModelHttp::dialectFromString("kiro"));
            });
        xwalk::hal::test::expectFailure(
            []()
            {
                static_cast<void>(XWalkHal::XWalkLanguageModelHttp::dialectFromString("unknown"));
            });
    }

    /**
     * @brief Verifies oldest-first history truncation and successful exchange
     * retention.
     */
    void testHistory()
    {
        TestTransport transport;
        XWalkHal::XWalkLanguageModelOllama backend(&transport, operations(), "http://localhost:11434/api/chat", "tiny");
        XWalkHal::XWalkLanguageModel model(&backend, backend.callbacks());
        model.setMaximumMessages(2U);
        model.addMessage(XWalkHal::XWalkLanguageModelRole::User, "discarded");
        model.addMessage(XWalkHal::XWalkLanguageModelRole::Assistant, "retained-one");
        model.addMessage(XWalkHal::XWalkLanguageModelRole::User, "retained-two");
        xwalk::hal::test::requireTestCondition(model.prompt("first") == "ready");
        xwalk::hal::test::requireTestCondition(transport.request.find("discarded") == XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("retained-one") != XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("retained-two") != XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(model.prompt("second") == "ready");
        xwalk::hal::test::requireTestCondition(transport.request.find("first") != XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("ready") != XWalkHal::string::npos);
        xwalk::hal::test::requireTestCondition(transport.request.find("retained-two") == XWalkHal::string::npos);
    }

    /**
     * @brief Verifies configuration, role, text, response, and request limits.
     */
    void testValidation()
    {
        TestTransport transport;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                const XWalkHal::XWalkLanguageModelOllamaOperations missing{};
                XWalkHal::XWalkLanguageModelOllama backend(
                    &transport, missing, "http://localhost:11434/api/chat", "tiny");
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkLanguageModelOllama backend(
                    &transport, operations(), "localhost:11434/api/chat", "tiny");
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkLanguageModelOllama backend(
                    &transport, operations(), "http://localhost:11434/api/chat", "", 0U);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkLanguageModelHttp backend(&transport,
                                                         operations(),
                                                         XWalkHal::XWalkLanguageModelHttpDialect::OpenAiChatCompletions,
                                                         "http://api.example/v1/chat/completions",
                                                         "model",
                                                         "key");
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkLanguageModelHttp backend(&transport,
                                                         operations(),
                                                         XWalkHal::XWalkLanguageModelHttpDialect::Ollama,
                                                         "http://localhost:11434/api/chat",
                                                         "model",
                                                         "unexpected-key");
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkLanguageModelHttp backend(&transport,
                                                         operations(),
                                                         XWalkHal::XWalkLanguageModelHttpDialect::OpenAiChatCompletions,
                                                         "https://api.example/v1/chat/completions",
                                                         "model",
                                                         "");
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkLanguageModelHttp backend(&transport,
                                                         operations(),
                                                         XWalkHal::XWalkLanguageModelHttpDialect::OpenAiChatCompletions,
                                                         "https://api.example/v1/chat/completions",
                                                         "model",
                                                         "bad\nkey");
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkLanguageModelHttp backend(&transport,
                                                         operations(),
                                                         XWalkHal::XWalkLanguageModelHttpDialect::OpenAiChatCompletions,
                                                         "https://api.example/v1/chat/completions",
                                                         "model",
                                                         "key",
                                                         5'000U,
                                                         0U);
            });

        XWalkHal::XWalkLanguageModelOllama backend(&transport, operations(), "http://localhost:11434/api/chat", "tiny");
        XWalkHal::XWalkLanguageModel model(&backend, backend.callbacks());
        xwalk::hal::test::expectFailure(
            [&]()
            {
                model.setMaximumMessages(XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_MESSAGES + 1U);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                model.addMessage(static_cast<XWalkHal::XWalkLanguageModelRole>(9U), "invalid");
            });
        const XWalkHal::string excessiveText(XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_TEXT_BYTES + 1U, 'x');
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(model.prompt(excessiveText));
            });
        transport.response.assign(XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_RESPONSE_BYTES + 1U, 'x');
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(model.prompt("response bound"));
            });

        transport.response = "{\"message\":{\"role\":\"assistant\",\"content\":"
                             "\"ready\"},\"done\":true}";
        model.setMaximumMessages(XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_MESSAGES);
        const XWalkHal::string boundedText(XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_TEXT_BYTES, 'y');
        for (XWalkHal::uint32 index = 0U; index < 33U; ++index)
        {
            model.addMessage(XWalkHal::XWalkLanguageModelRole::User, boundedText);
        }
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(model.prompt("request bound"));
            });
    }

    /**
     * @brief Verifies transport and malformed-response failures are propagated.
     */
    void testFailures()
    {
        TestTransport transport;
        XWalkHal::XWalkLanguageModelOllama backend(&transport, operations(), "http://localhost:11434/api/chat", "tiny");
        XWalkHal::XWalkLanguageModel model(&backend, backend.callbacks());
        transport.fail = true;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(model.prompt("transport"));
            });
        transport.fail = false;
        transport.response = "{\"done\":true}";
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(model.prompt("missing content"));
            });
        transport.response = "{\"message\":{\"content\":\"\\uD800\"}}";
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(model.prompt("invalid unicode"));
            });
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every device-free Ollama provider test.
 *
 * @param[in] argumentCount Exactly two arguments are required.
 * @param[in] argumentValues Program name and test-owned output directory.
 * @return Zero after every assertion passes.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer argumentValues[])
{
    XWalkHal::XWalkTrace::configureGlobal(XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_CONFIG_PATH,
                                          XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .155, "HTTP language-model provider tests started");
    xwalk::hal::test::requireTestCondition(argumentCount == 2);
    const XWalkHal::filesystempath imagePath = writeImageFixture(argumentValues[1]);
    testRequestAndResponse(imagePath);
    testOpenAiCompatibleRequest(imagePath);
    testProviderNames();
    testHistory();
    testValidation();
    testFailures();
    XWALK_HAL_TRACE_UID0(RPI .156, "HTTP language-model provider tests completed");
    return 0;
}
