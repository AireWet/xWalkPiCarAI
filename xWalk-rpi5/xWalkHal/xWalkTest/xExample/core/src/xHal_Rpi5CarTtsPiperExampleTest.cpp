/******************************************************************************
 * @file        xHal_Rpi5CarTtsPiperExampleTest.cpp
 * @brief       Verifies Piper flow without process or audio access.
 *
 * @details
 * Records the injected request to verify exact model and message
 * preservation and confirms that a missing speech operation is rejected.
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

#include "xHal_Rpi5CarTtsPiperExample.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>
#include "xHal_Rpi5CarTtsPiperExampleTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TtsPiperExampleState = ::xwalk::source_types::xhal_rpi5carttspiperexampletest::TtsPiperExampleState;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains host-test state and scenarios private to this source. */
namespace
{

    /**
     * @brief Records one injected Piper speech request.
     * @param[in,out] context Non-null pointer to a live test state.
     * @param[in] model Voice-model name copied into the test state.
     * @param[in] text Speech text copied into the test state.
     */
    void speak(XWalkHal::contextpointer context, XWalkHal::stringview model, XWalkHal::stringview text)
    {
        TtsPiperExampleState& state = *static_cast<TtsPiperExampleState*>(context);
        state.model = model;
        state.text = text;
        ++state.callCount;
    }

    /** @brief Verifies the exact model, message, and single invocation. */
    void testRequest()
    {
        TtsPiperExampleState state;
        xwalk::hal::example::XWalkTtsPiperExample example(&state, &speak);

        example.run();

        assert(state.callCount == 1U);
        assert(state.model == "en_US-amy-low");
        assert(state.text == "Hi, I'm piper TTS. A fast and local neural text-to-speech engine that "
                             "embeds espeak-ng for phonemization.");
    }

    /** @brief Verifies rejection of a missing speech operation. */
    void testValidation()
    {
        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::hal::example::XWalkTtsPiperExample invalid(nullptr, nullptr);
            });
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe Piper example verification.
 * @return Zero after all assertions pass.
 */
int xWalkTtsPiperExampleHostTest()
{
    testRequest();
    testValidation();
    return 0;
}
