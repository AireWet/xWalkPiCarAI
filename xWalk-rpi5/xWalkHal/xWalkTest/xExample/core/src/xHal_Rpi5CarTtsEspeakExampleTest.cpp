/******************************************************************************
 * @file        xHal_Rpi5CarTtsEspeakExampleTest.cpp
 * @brief       Verifies Espeak settings without process or audio access.
 *
 * @details
 * Records one injected request to verify every exact setting and message and
 * confirms that a missing speech operation is rejected.
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

#include "xHal_Rpi5CarTtsEspeakExample.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>
#include "xHal_Rpi5CarTtsEspeakExampleTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TtsEspeakExampleState = ::xwalk::source_types::xhal_rpi5carttsespeakexampletest::TtsEspeakExampleState;

namespace
{

    /** @brief Records one injected Espeak request. */
    void speak(XWalkHal::contextpointer context,
               XWalkHal::uint8 amplitude,
               XWalkHal::uint16 speed,
               XWalkHal::uint16 gap,
               XWalkHal::uint8 pitch,
               XWalkHal::stringview text)
    {
        TtsEspeakExampleState& state = *static_cast<TtsEspeakExampleState*>(context);
        state.amplitude = amplitude;
        state.speed = speed;
        state.gap = gap;
        state.pitch = pitch;
        state.text = text;
        ++state.callCount;
    }

    /** @brief Verifies every exact setting, message, and single invocation. */
    void testRequest()
    {
        TtsEspeakExampleState state;
        xwalk::hal::example::XWalkTtsEspeakExample example(&state, &speak);

        example.run();

        assert(state.callCount == 1U);
        assert(state.amplitude == 100U);
        assert(state.speed == 150U);
        assert(state.gap == 1U);
        assert(state.pitch == 80U);
        assert(state.text == "Hello world!");
    }

    /** @brief Verifies rejection of a missing speech operation. */
    void testValidation()
    {
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::example::XWalkTtsEspeakExample invalid(nullptr, nullptr);
            });
    }

} /* namespace */

/** @brief Runs the host-safe configured Espeak example verification. */
int xWalkTtsEspeakExampleHostTest()
{
    testRequest();
    testValidation();
    return 0;
}
