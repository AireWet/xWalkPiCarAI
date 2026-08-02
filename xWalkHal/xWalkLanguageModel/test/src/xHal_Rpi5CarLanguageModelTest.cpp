/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelTest.cpp
 * @brief       Verifies language-model dispatch and validation behavior.
 *
 * @details
 * Exercises conversation configuration, default and explicit history limits,
 * message roles, optional image paths, prompting, and backend failures.
 *
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Test
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModel.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains host-test state and callbacks private to this translation unit.
 */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Supplies deterministic language-model callback behavior. */
struct TestLanguageModelBackend
{
    string instructions{}; /**< Most recently supplied system instructions. */
    string welcome{}; /**< Most recently supplied welcome text. */
    string messageContent{}; /**< Most recently added conversation content. */
    string messageImagePath{}; /**< Most recently added message image path. */
    string promptText{}; /**< Most recently submitted prompt text. */
    string promptImagePath{}; /**< Most recently submitted prompt image path. */
    string promptResult{"model response"}; /**< Next final response returned by the backend. */
    XWalkLanguageModelRole role{XWalkLanguageModelRole::System}; /**< Most recent message role. */
    uint32 maximumMessages{}; /**< Most recently supplied retained-message limit. */
    uint32 instructionCount{}; /**< Number of instruction callback entries. */
    uint32 welcomeCount{}; /**< Number of welcome callback entries. */
    uint32 limitCount{}; /**< Number of retained-message callback entries. */
    uint32 messageCount{}; /**< Number of message callback entries. */
    uint32 promptCount{}; /**< Number of prompt callback entries. */
    boolean failPrompt{}; /**< `true` to fail the next prompt request. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Records system instructions supplied by the coordinator.
 *
 * @param[in,out] context
 * Non-null test language-model backend.
 *
 * @param[in] instructions
 * System instructions to retain for assertions.
 */
void setInstructions(contextpointer context, stringview instructions)
{
    TestLanguageModelBackend& backend = *static_cast<TestLanguageModelBackend*>(context);
    ++backend.instructionCount;
    backend.instructions = string(instructions);
}

/**
 * @brief Records welcome text supplied by the coordinator.
 *
 * @param[in,out] context
 * Non-null test language-model backend.
 *
 * @param[in] welcome
 * Welcome text to retain for assertions.
 */
void setWelcome(contextpointer context, stringview welcome)
{
    TestLanguageModelBackend& backend = *static_cast<TestLanguageModelBackend*>(context);
    ++backend.welcomeCount;
    backend.welcome = string(welcome);
}

/**
 * @brief Records a retained conversation-message limit.
 *
 * @param[in,out] context
 * Non-null test language-model backend.
 *
 * @param[in] maximumMessages
 * Validated non-zero retained-message limit.
 */
void setMaximumMessages(contextpointer context, uint32 maximumMessages)
{
    TestLanguageModelBackend& backend = *static_cast<TestLanguageModelBackend*>(context);
    ++backend.limitCount;
    backend.maximumMessages = maximumMessages;
}

/**
 * @brief Records one conversation message and its optional image path.
 *
 * @param[in,out] context
 * Non-null test language-model backend.
 *
 * @param[in] role
 * Participant responsible for the message.
 *
 * @param[in] content
 * Message content to retain for assertions.
 *
 * @param[in] imagePath
 * Optional image path to retain for assertions.
 */
void addMessage(contextpointer context, XWalkLanguageModelRole role, stringview content,
    stringview imagePath)
{
    TestLanguageModelBackend& backend = *static_cast<TestLanguageModelBackend*>(context);
    ++backend.messageCount;
    backend.role = role;
    backend.messageContent = string(content);
    backend.messageImagePath = string(imagePath);
}

/**
 * @brief Records one prompt and returns the configured final response.
 *
 * @param[in,out] context
 * Non-null test language-model backend.
 *
 * @param[in] promptText
 * Prompt text to retain for assertions.
 *
 * @param[in] imagePath
 * Optional image path to retain for assertions.
 *
 * @return
 * Configured final language-model response.
 *
 * @throws std::runtime_error
 * If prompt failure is enabled.
 */
string prompt(contextpointer context, stringview promptText, stringview imagePath)
{
    TestLanguageModelBackend& backend = *static_cast<TestLanguageModelBackend*>(context);
    ++backend.promptCount;
    backend.promptText = string(promptText);
    backend.promptImagePath = string(imagePath);
    if (backend.failPrompt)
    {
        XHAL_THROW_RUNTIME_ERROR("Test language-model prompt failed");
    }
    return backend.promptResult;
}

/**
 * @brief Returns the complete in-memory language-model backend callback table.
 *
 * @return
 * Callback table containing only non-null functions.
 */
XWalkLanguageModelCallbacks backendCallbacks()
{
    return {&setInstructions, &setWelcome, &setMaximumMessages, &addMessage, &prompt};
}

/** @brief Verifies conversation configuration and message dispatch. */
void testConversationConfiguration()
{
    TestLanguageModelBackend backend;
    XWalkLanguageModel languageModel(&backend, backendCallbacks());

    languageModel.setInstructions("Be concise");
    assert(backend.instructions == "Be concise");
    assert(backend.instructionCount == 1U);

    languageModel.setWelcome("Welcome");
    assert(backend.welcome == "Welcome");
    assert(backend.welcomeCount == 1U);

    languageModel.setMaximumMessages();
    assert(backend.maximumMessages == XHAL_RPI5CAR_LANGUAGE_MODEL_DEFAULT_MAXIMUM_MESSAGES);
    languageModel.setMaximumMessages(35U);
    assert(backend.maximumMessages == 35U);
    assert(backend.limitCount == 2U);

    languageModel.addMessage(XWalkLanguageModelRole::User, "Inspect this", "frame.jpg");
    assert(backend.role == XWalkLanguageModelRole::User);
    assert(backend.messageContent == "Inspect this");
    assert(backend.messageImagePath == "frame.jpg");
    assert(backend.messageCount == 1U);
}

/** @brief Verifies text-only, image-assisted, and empty model responses. */
void testPrompting()
{
    TestLanguageModelBackend backend;
    XWalkLanguageModel languageModel(&backend, backendCallbacks());

    assert(languageModel.prompt("Hello") == "model response");
    assert(backend.promptText == "Hello");
    assert(backend.promptImagePath.empty());

    backend.promptResult.clear();
    assert(languageModel.prompt("Describe", "frame.jpg").empty());
    assert(backend.promptImagePath == "frame.jpg");
    assert(backend.promptCount == 2U);

    languageModel.setInstructions("");
    languageModel.setWelcome("");
    languageModel.addMessage(XWalkLanguageModelRole::Assistant, "");
    assert(backend.instructions.empty());
    assert(backend.welcome.empty());
    assert(backend.messageContent.empty());
    assert(backend.messageImagePath.empty());
}

/** @brief Verifies a zero retained-message limit is rejected before dispatch. */
void testLimitValidation()
{
    TestLanguageModelBackend backend;
    XWalkLanguageModel languageModel(&backend, backendCallbacks());

    xwalk::hal::test::expectFailure([&]()
    {
        languageModel.setMaximumMessages(0U);
    });
    assert(backend.limitCount == 0U);
}

/** @brief Verifies prompt failures are propagated without response substitution. */
void testBackendFailure()
{
    TestLanguageModelBackend backend;
    backend.failPrompt = true;
    XWalkLanguageModel languageModel(&backend, backendCallbacks());

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(languageModel.prompt("fail"));
    });
}

/** @brief Verifies construction rejects every incomplete callback-table shape. */
void testCallbackValidation()
{
    const fixedarray<XWalkLanguageModelCallbacks, 5U> incompleteCallbacks{
        XWalkLanguageModelCallbacks{nullptr, &setWelcome, &setMaximumMessages, &addMessage, &prompt},
        XWalkLanguageModelCallbacks{&setInstructions, nullptr, &setMaximumMessages, &addMessage, &prompt},
        XWalkLanguageModelCallbacks{&setInstructions, &setWelcome, nullptr, &addMessage, &prompt},
        XWalkLanguageModelCallbacks{&setInstructions, &setWelcome, &setMaximumMessages, nullptr, &prompt},
        XWalkLanguageModelCallbacks{&setInstructions, &setWelcome, &setMaximumMessages, &addMessage,
            nullptr}};

    TestLanguageModelBackend backend;
    for (const XWalkLanguageModelCallbacks& callbacks : incompleteCallbacks)
    {
        xwalk::hal::test::expectFailure([&]()
        {
            XWalkLanguageModel languageModel(&backend, callbacks);
        });
    }
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every host-side language-model test.
 *
 * @return
 * Zero after every assertion succeeds.
 */
int main()
{
    testConversationConfiguration();
    testPrompting();
    testLimitValidation();
    testBackendFailure();
    testCallbackValidation();
    return 0;
}
