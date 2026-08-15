/******************************************************************************
 * @file        xHal_Rpi5CarTtsEdgeExampleTest.cpp
 * @brief       Verifies Edge TTS request flow without network or audio access.
 *
 * @details
 * Records the injected request to verify exact voice and message preservation
 * and confirms that a missing speech operation is rejected.
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

#include "xHal_Rpi5CarTtsEdgeExample.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>
#include "xHal_Rpi5CarTtsEdgeExampleTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TtsEdgeExampleState = ::xwalk::source_types::xhal_rpi5carttsedgeexampletest::TtsEdgeExampleState;

namespace
{

    /** @brief Records one injected Edge TTS request. */
    void speak(XWalkHal::contextpointer context, XWalkHal::stringview voice, XWalkHal::stringview text)
    {
        TtsEdgeExampleState& state = *static_cast<TtsEdgeExampleState*>(context);
        state.voice = voice;
        state.text = text;
        ++state.callCount;
    }

    /** @brief Verifies the exact voice, message, and single invocation. */
    void testRequest()
    {
        TtsEdgeExampleState state;
        xwalk::hal::example::XWalkTtsEdgeExample example(&state, &speak);

        example.run();

        assert(state.callCount == 1U);
        assert(state.voice == "en-US-AriaNeural");
        assert(state.text == "Hi, I'm Edge TTS. A free cloud text-to-speech service powered by Microsoft Edge.");
    }

    /** @brief Verifies rejection of a missing speech operation. */
    void testValidation()
    {
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::example::XWalkTtsEdgeExample invalid(nullptr, nullptr);
            });
    }

} /* namespace */

/** @brief Runs the host-safe Edge TTS example verification. */
int xWalkTtsEdgeExampleHostTest()
{
    testRequest();
    testValidation();
    return 0;
}
