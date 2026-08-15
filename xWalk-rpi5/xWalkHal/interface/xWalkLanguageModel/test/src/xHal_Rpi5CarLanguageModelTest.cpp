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
#include "xHal_Rpi5CarLanguageModelSimulationArguments.h"
#include "xHal_Rpi5CarLanguageModelSimulationConfig.h"
#include "xHal_Rpi5CarLanguageModelTestSupport.h"

#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains host-test state and callbacks private to this translation
 * unit.
 */
namespace
{

    using namespace xwalk::hal;
    using namespace xwalk::hal::test::language_model;

    /******************************************************************************
     * Private function definitions
     ******************************************************************************/

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

    /** @brief Verifies a zero retained-message limit is rejected before dispatch.
     */
    void testLimitValidation()
    {
        TestLanguageModelBackend backend;
        XWalkLanguageModel languageModel(&backend, backendCallbacks());

        xwalk::hal::test::expectFailure(
            [&]()
            {
                languageModel.setMaximumMessages(0U);
            });
        assert(backend.limitCount == 0U);
    }

    /** @brief Verifies prompt failures are propagated without response
     * substitution. */
    void testBackendFailure()
    {
        TestLanguageModelBackend backend;
        backend.failPrompt = true;
        XWalkLanguageModel languageModel(&backend, backendCallbacks());

        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(languageModel.prompt("fail"));
            });
    }

    /** @brief Verifies construction rejects every incomplete callback-table shape.
     */
    void testCallbackValidation()
    {
        const fixedarray<XWalkLanguageModelCallbacks, 5U> incompleteCallbacks{
            XWalkLanguageModelCallbacks{nullptr, &setWelcome, &setMaximumMessages, &addMessage, &prompt},
            XWalkLanguageModelCallbacks{&setInstructions, nullptr, &setMaximumMessages, &addMessage, &prompt},
            XWalkLanguageModelCallbacks{&setInstructions, &setWelcome, nullptr, &addMessage, &prompt},
            XWalkLanguageModelCallbacks{&setInstructions, &setWelcome, &setMaximumMessages, nullptr, &prompt},
            XWalkLanguageModelCallbacks{&setInstructions, &setWelcome, &setMaximumMessages, &addMessage, nullptr}};

        TestLanguageModelBackend backend;
        for (const XWalkLanguageModelCallbacks& callbacks : incompleteCallbacks)
        {
            xwalk::hal::test::expectFailure(
                [&]()
                {
                    XWalkLanguageModel languageModel(&backend, callbacks);
                });
        }
    }

    /** @brief Verifies persistent simulation-selector parsing. */
    void testSimulationArguments()
    {
        char binaryName[] = "xWalkLanguageModelSimulation";
        char traceOption[] = "--trace";
        char enableSelector[] = "RPI.150.enable";
        charpointer enableValues[]{binaryName, traceOption, enableSelector};
        const sim::XWalkLanguageModelSimulationArguments enable(3, enableValues);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());
        char disableSelector[] = "RPI.150.disable";
        charpointer disableValues[]{binaryName, traceOption, disableSelector};
        const sim::XWalkLanguageModelSimulationArguments disable(3, disableValues);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());
        char malformedSelector[] = "RPI.Model.enable";
        charpointer malformedValues[]{binaryName, traceOption, malformedSelector};
        const sim::XWalkLanguageModelSimulationArguments malformed(3, malformedValues);
        assert(malformed.valid() == false);
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
    XWalkTrace::configureGlobal(XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_CONFIG_PATH,
                                XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .153, "xWalkLanguageModel host tests started");
    testConversationConfiguration();
    testPrompting();
    testLimitValidation();
    testBackendFailure();
    testCallbackValidation();
    testSimulationArguments();
    XWALK_HAL_TRACE_UID0(RPI .154, "xWalkLanguageModel host tests completed");
    return 0;
}
