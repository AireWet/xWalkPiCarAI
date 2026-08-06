/******************************************************************************
 * @file        xControllerSequenceTest.cpp
 * @brief       Verifies bounded CLI command sequencing with in-memory callbacks.
 *
 * @details
 * Exercises ordered success, first-failure termination, unavailable backend
 * propagation, and complete pre-execution validation without hardware access.
 *
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test Host Test
 *
 * @author      Joxy John
 * @date        2026-08-04
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

#include "xControllerSequence.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory CLI backend and host scenario. */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records every output line emitted by the simulated CLI backend. */
struct TestState
{
    /** @brief Output lines retained in exact callback order. */
    ctrl::stringvector outputLines;
};

/******************************************************************************
 * Private callback definitions
 ******************************************************************************/

/**
 * @brief Records one synchronous CLI output line.
 *
 * @param[in,out] context
 * Non-null `TestState` pointer.
 *
 * @param[in] line
 * Output text copied before this callback returns.
 */
void output(ctrl::contextpointer context, ctrl::stringview line)
{
    static_cast<TestState*>(context)->outputLines.emplace_back(line);
}

/**
 * @brief Supplies an empty deterministic response.
 *
 * @param[in,out] context
 * Test context; unused.
 *
 * @param[in] prompt
 * Prompt text; unused.
 *
 * @return
 * Empty owned text.
 */
ctrl::string input(ctrl::contextpointer context, ctrl::stringview prompt)
{
    static_cast<void>(context);
    static_cast<void>(prompt);
    return {};
}

/**
 * @brief Accepts one simulated delay without waiting.
 *
 * @param[in,out] context
 * Test context; unused.
 *
 * @param[in] durationMs
 * Delay in milliseconds; unused.
 */
void delay(ctrl::contextpointer context, ctrl::uint32 durationMs)
{
    static_cast<void>(context);
    static_cast<void>(durationMs);
}

/**
 * @brief Permits every bounded simulated operation.
 *
 * @param[in,out] context
 * Test context; unused.
 *
 * @return
 * Always `true`.
 */
ctrl::boolean continueOperation(ctrl::contextpointer context)
{
    static_cast<void>(context);
    return true;
}

/**
 * @brief Accepts one simulated sound request.
 *
 * @param[in,out] context
 * Test context; unused.
 *
 * @param[in] request
 * Sound operation, path, and optional volume; unused.
 *
 * @return
 * Always `true`.
 */
ctrl::boolean sound(ctrl::contextpointer context,
    const xwalk::ctrl::XWalkSoundRequest& request)
{
    static_cast<void>(context);
    static_cast<void>(request);
    return true;
}

/**
 * @brief Returns the complete simulated CLI callback table.
 *
 * @return
 * Non-null callback table whose functions do not access hardware.
 */
xwalk::ctrl::XWalkControllerCallbacks callbacks()
{
    return {&output, &input, &delay, &continueOperation, &sound};
}

/******************************************************************************
 * Test function definitions
 ******************************************************************************/

/**
 * @brief Exercises successful, failed, unavailable, and invalid sequences.
 *
 * @post
 * Every command result, output boundary, and validation assertion passes.
 */
void runTest()
{
    TestState passingState;
    const ctrl::stringvector passingReport{
        "=== PiCar-X Passive Hardware Preflight ===",
        "[PASS] Configuration: ready"};
    xwalk::ctrl::XWalkController passingController(
        passingReport, &passingState, callbacks());
    xwalk::agent::test::XWalkControllerSequence passingSequence(passingController);
    const xwalk::agent::test::controllercommandsequence passingCommands{
        {"help"}, {"doctor"}, {"--help"}};
    assert(passingSequence.run(passingCommands) == 0);
    assert(passingState.outputLines.size() == 4U);
    assert(passingState.outputLines[1U] == passingReport[0U]);
    assert(passingState.outputLines[2U] == passingReport[1U]);

    TestState failingState;
    const ctrl::stringvector failingReport{"[FAIL] I2C: unavailable"};
    xwalk::ctrl::XWalkController failingController(
        failingReport, &failingState, callbacks());
    xwalk::agent::test::XWalkControllerSequence failingSequence(failingController);
    const xwalk::agent::test::controllercommandsequence failingCommands{
        {"help"}, {"doctor"}, {"help"}};
    assert(failingSequence.run(failingCommands) == 2);
    assert(failingState.outputLines.size() == 2U);
    assert(failingState.outputLines.back() == failingReport.front());

    TestState unavailableState;
    xwalk::ctrl::XWalkController unavailableController(
        passingReport, &unavailableState, callbacks());
    xwalk::agent::test::XWalkControllerSequence unavailableSequence(unavailableController);
    assert(unavailableSequence.run({{"move", "forward"}}) == 3);
    assert(unavailableState.outputLines.back() == "PiCar-X backend unavailable");

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(passingSequence.run({}));
    });
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(passingSequence.run({{"help"}, {}}));
    });
    const xwalk::agent::test::controllercommandsequence oversizedCommands(
        33U, ctrl::stringvector{"help"});
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(passingSequence.run(oversizedCommands));
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe CLI controller-sequence verification.
 *
 * @return
 * Zero after every assertion passes.
 */
int xWalkControllerSequenceHostTest()
{
    runTest();
    return 0;
}
