/******************************************************************************
 * @file        xHal_Rpi5CarTtsPico2WaveExampleTest.cpp
 * @brief       Verifies Pico2Wave flow without process or audio access.
 *
 * @details
 * Records the injected request to verify exact language and message
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

#include "xHal_Rpi5CarTtsPico2WaveExample.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains host-test state and scenarios private to this source. */
namespace
{

/** @brief Records one deterministic language and speech message. */
struct TtsPico2WaveExampleState
{
    /** @brief Owned language supplied to the speech callback. */
    XWalkHal::string language;

    /** @brief Owned message supplied to the speech callback. */
    XWalkHal::string text;

    /** @brief Number of observed synchronous speech requests. */
    XWalkHal::uint32 callCount{};
};

/**
 * @brief Records one injected Pico2Wave speech request.
 * @param[in,out] context Non-null pointer to a live test state.
 * @param[in] language Language copied into the test state.
 * @param[in] text Speech text copied into the test state.
 */
void speak(XWalkHal::contextpointer context, XWalkHal::stringview language,
    XWalkHal::stringview text)
{
    TtsPico2WaveExampleState& state =
        *static_cast<TtsPico2WaveExampleState*>(context);
    state.language = language;
    state.text = text;
    ++state.callCount;
}

/** @brief Verifies the exact language, message, and single invocation. */
void testRequest()
{
    TtsPico2WaveExampleState state;
    xwalk::hal::example::XWalkTtsPico2WaveExample example(&state, &speak);

    example.run();

    assert(state.callCount == 1U);
    assert(state.language == "en-US");
    assert(state.text == "Hello world!");
}

/** @brief Verifies rejection of a missing speech operation. */
void testValidation()
{
    XWalkHal::boolean rejected = false;
    try
    {
        xwalk::hal::example::XWalkTtsPico2WaveExample invalid(nullptr, nullptr);
    }
    catch (const XWalkHal::invalidargument&)
    {
        rejected = true;
    }
    assert(rejected);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe Pico2Wave example verification.
 * @return Zero after all assertions pass.
 */
int xWalkTtsPico2WaveExampleHostTest()
{
    testRequest();
    testValidation();
    return 0;
}
