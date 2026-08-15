/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordExampleTest.cpp
 * @brief       Verifies synchronous wake detection without microphone access.
 *
 * @details
 * Uses deterministic transcripts to verify prompting, case-insensitive wake
 * matching, attempt and timeout validation, and bounded unsuccessful waits.
 *
 * @project     xWalk Firmware
 * @module      xExample Host Test
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskWakeWordExample.h"

#include <cassert>
#include "xHal_Rpi5CarSttVoskWakeWordExampleTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using WakeWordExampleState = ::xwalk::source_types::xhal_rpi5carsttvoskwakewordexampletest::WakeWordExampleState;

namespace
{

    /** @brief Returns one configured transcript and records its timeout. */
    XWalkHal::string listen(XWalkHal::contextpointer context, XWalkHal::uint32 timeoutMs)
    {
        WakeWordExampleState& state = *static_cast<WakeWordExampleState*>(context);
        state.timeouts.push_back(timeoutMs);
        const XWalkHal::string transcript = state.transcripts[state.transcriptIndex];
        ++state.transcriptIndex;
        return transcript;
    }

    /** @brief Records one literal status message. */
    void report(XWalkHal::contextpointer context, XWalkHal::stringview message)
    {
        static_cast<WakeWordExampleState*>(context)->messages.emplace_back(message);
    }

    /** @brief Returns the complete deterministic callback table. */
    xwalk::hal::example::XWalkSttVoskWakeWordExampleCallbacks callbacks()
    {
        return {&listen, &report};
    }

    /** @brief Verifies prompt, repeated listen, case folding, and detection output. */
    void testWakeDetection()
    {
        WakeWordExampleState state;
        xwalk::hal::example::XWalkSttVoskWakeWordExample example(&state, callbacks());

        example.run(2U, 4'000U);

        assert(state.transcriptIndex == 2U);
        assert(state.timeouts == XWalkHal::uint32vector({4'000U, 4'000U}));
        assert(state.messages == XWalkHal::stringvector({"Wake me with :\"Hey robot\"", "Wake word detected"}));
    }

    /** @brief Verifies callback, attempt, and timeout validation. */
    void testValidation()
    {
        WakeWordExampleState state;
        auto incomplete = callbacks();
        incomplete.listen = nullptr;
        XWalkHal::boolean rejectedCallbacks = false;
        try
        {
            xwalk::hal::example::XWalkSttVoskWakeWordExample invalid(&state, incomplete);
        }
        catch (const XWalkHal::invalidargument&)
        {
            rejectedCallbacks = true;
        }
        assert(rejectedCallbacks);

        xwalk::hal::example::XWalkSttVoskWakeWordExample example(&state, callbacks());
        XWalkHal::boolean rejectedAttempts = false;
        XWalkHal::boolean rejectedTimeout = false;
        try
        {
            example.run(0U, 1U);
        }
        catch (const XWalkHal::outofrange&)
        {
            rejectedAttempts = true;
        }
        try
        {
            example.run(1U, XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS + 1U);
        }
        catch (const XWalkHal::outofrange&)
        {
            rejectedTimeout = true;
        }
        assert(rejectedAttempts);
        assert(rejectedTimeout);
    }

    /** @brief Verifies the bounded failure when no transcript contains the phrase. */
    void testAttemptLimit()
    {
        WakeWordExampleState state;
        state.transcripts = {"noise", "silence"};
        xwalk::hal::example::XWalkSttVoskWakeWordExample example(&state, callbacks());
        XWalkHal::boolean rejected = false;
        try
        {
            example.run(2U, 1U);
        }
        catch (const XWalkHal::runtimeerror&)
        {
            rejected = true;
        }
        assert(rejected);
        assert(state.transcriptIndex == 2U);
    }

} /* namespace */

/** @brief Runs the host-safe synchronous wake-word example verification. */
int xWalkSttVoskWakeWordExampleHostTest()
{
    testWakeDetection();
    testValidation();
    testAttemptLimit();
    return 0;
}
