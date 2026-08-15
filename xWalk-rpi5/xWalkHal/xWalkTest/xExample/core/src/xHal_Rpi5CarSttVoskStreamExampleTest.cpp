/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskStreamExampleTest.cpp
 * @brief       Verifies streaming Vosk example flow without microphone access.
 *
 * @details
 * Uses deterministic callbacks to verify prompts, partial and final results,
 * session bounds, timeout bounds, and callback-table validation.
 *
 * @project     xWalk Firmware
 * @module      xExample Host Test
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarSttVoskStreamExample.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>
#include "xHal_Rpi5CarSttVoskStreamExampleTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using SttVoskStreamExampleState =
    ::xwalk::source_types::xhal_rpi5carsttvoskstreamexampletest::SttVoskStreamExampleState;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

    /** @brief Records one streamed result using its source-compatible label. */
    void reportResult(XWalkHal::contextpointer context, XWalkHal::boolean done, XWalkHal::stringview text)
    {
        SttVoskStreamExampleState& state = *static_cast<SttVoskStreamExampleState*>(context);
        state.results.push_back((done ? "final:   " : "partial: ") + XWalkHal::string(text));
    }

    /** @brief Emits one partial result followed by one final result. */
    void listen(XWalkHal::contextpointer context,
                XWalkHal::uint32 timeoutMs,
                xwalk::hal::example::sttvoskstreamresultcallback resultCallback)
    {
        SttVoskStreamExampleState& state = *static_cast<SttVoskStreamExampleState*>(context);
        ++state.listenCount;
        state.timeoutMs = timeoutMs;
        resultCallback(context, false, "hello");
        resultCallback(context, true, "hello robot");
    }

    /** @brief Records one source prompt. */
    void reportPrompt(XWalkHal::contextpointer context)
    {
        ++static_cast<SttVoskStreamExampleState*>(context)->promptCount;
    }

    /** @brief Returns the complete deterministic operation table. */
    xwalk::hal::example::XWalkSttVoskStreamExampleCallbacks callbacks()
    {
        return {&listen, &reportPrompt, &reportResult};
    }

    /** @brief Verifies bounded prompts and partial/final result forwarding. */
    void testStreamingFlow()
    {
        SttVoskStreamExampleState state;
        xwalk::hal::example::XWalkSttVoskStreamExample example(&state, callbacks());

        example.run(2U, 5'000U);

        assert(state.promptCount == 2U);
        assert(state.listenCount == 2U);
        assert(state.timeoutMs == 5'000U);
        assert(state.results ==
               XWalkHal::stringvector(
                   {"partial: hello", "final:   hello robot", "partial: hello", "final:   hello robot"}));
    }

    /** @brief Verifies callback, session-count, and timeout validation. */
    void testValidation()
    {
        SttVoskStreamExampleState state;
        xwalk::hal::example::XWalkSttVoskStreamExampleCallbacks incomplete = callbacks();
        incomplete.listen = nullptr;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::example::XWalkSttVoskStreamExample invalidExample(&state, incomplete);
            });

        xwalk::hal::example::XWalkSttVoskStreamExample example(&state, callbacks());
        xwalk::hal::test::expectFailure(
            [&]()
            {
                example.run(0U, 1U);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                example.run(1U, XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS + 1U);
            });
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/** @brief Runs the host-safe streaming Vosk example verification. */
int xWalkSttVoskStreamExampleHostTest()
{
    testStreamingFlow();
    testValidation();
    return 0;
}
