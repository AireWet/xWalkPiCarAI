/******************************************************************************
 * @file        xAgent_Rpi5CarServoZeroingTest.cpp
 * @brief       Verifies servo-zeroing order without physical PWM hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkServoZeroing Host Test
 *
 * @author      Joxy John
 * @date        2026-08-05
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

#include "xAgent_Rpi5CarServoZeroing.h"

#include <cassert>
#include "xAgent_Rpi5CarServoZeroingTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TestState = ::xwalk::source_types::xagent_rpi5carservozeroingtest::TestState;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains deterministic host-test support local to this translation unit.
 */
namespace
{

    /** @brief Records one logical servo command. */
    void setAngle(xwalk::agent::contextpointer context, xwalk::agent::uint8 servoId, xwalk::agent::float64 angleDegrees)
    {
        TestState& state = *static_cast<TestState*>(context);
        state.servoIds.push_back(static_cast<xwalk::agent::uint32>(servoId));
        state.angles.push_back(angleDegrees);
    }

    /** @brief Records one cancellable delay slice. */
    void delay(xwalk::agent::contextpointer context, xwalk::agent::uint32 durationMs)
    {
        static_cast<TestState*>(context)->delays.push_back(durationMs);
    }

    /** @brief Cancels after the complete 24-command sequence reaches its idle wait. */
    xwalk::agent::boolean continueOperation(xwalk::agent::contextpointer context)
    {
        TestState& state = *static_cast<TestState*>(context);
        ++state.continueQueries;
        return state.continueQueries <= 120U;
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs deterministic source-order assertions.
 * @return Zero after every assertion passes.
 */
int main()
{
    TestState state;
    const xwalk::agent::XWalkServoZeroingCallbacks callbacks{&setAngle, &delay, &continueOperation};
    xwalk::agent::XWalkServoZeroing example(&state, callbacks);
    const xwalk::agent::boolean result = example.run();
    assert(result);
    assert(state.servoIds.size() == 24U);
    assert(state.angles.size() == 24U);
    for (xwalk::agent::uint8 servoId = 0U; servoId < xwalk::agent::XAGENT_RPI5CAR_SERVO_ZEROING_CHANNEL_COUNT;
         ++servoId)
    {
        const xwalk::agent::size index = static_cast<xwalk::agent::size>(servoId) * 2U;
        assert(state.servoIds[index] == servoId);
        assert(state.servoIds[index + 1U] == servoId);
        assert(state.angles[index] == 10.0);
        assert(state.angles[index + 1U] == 0.0);
    }
    assert(!state.delays.empty());
    for (const xwalk::agent::uint32 durationMs : state.delays)
    {
        assert(durationMs == 20U);
    }
    return 0;
}
