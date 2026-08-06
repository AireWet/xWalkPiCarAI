/******************************************************************************
 * @file        xAgent_Rpi5CarComputerVisionTest.cpp
 * @brief       Verifies computer-vision keys through deterministic callbacks.
 *
 * @details
 * Covers provider lifecycle, every source key group, retained detector modes,
 * QR change reporting, object observations, timing, cancellation, and validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkComputerVision Host Test
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

#include "xAgent_Rpi5CarComputerVision.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains deterministic provider state and host-test callbacks. */
namespace
{

/** @brief Records every observable provider operation. */
struct TestState
{
    agent::boolean started{};
    agent::uint32 stopCount{};
    xwalk::agent::XWalkComputerVisionColor color{
        xwalk::agent::XWalkComputerVisionColor::Close};
    agent::boolean faceEnabled{};
    agent::boolean qrEnabled{};
    agent::uint32 delayTotalMs{};
    agent::uint32 continueCount{};
    agent::uint32 continueLimit{1'000U};
};

/** @brief Starts the simulated camera provider. */
agent::boolean start(agent::contextpointer context)
{
    static_cast<TestState*>(context)->started = true;
    return true;
}

/** @brief Stops the simulated camera provider. */
void stop(agent::contextpointer context) noexcept
{
    TestState& state = *static_cast<TestState*>(context);
    state.started = false;
    state.color = xwalk::agent::XWalkComputerVisionColor::Close;
    state.faceEnabled = false;
    state.qrEnabled = false;
    ++state.stopCount;
}

/** @brief Returns one deterministic photograph path. */
agent::string capture(agent::contextpointer context)
{
    static_cast<void>(context);
    return "/tmp/photo_2026-08-04-12-00-00.jpg";
}

/** @brief Records the selected color. */
void setColor(agent::contextpointer context,
    xwalk::agent::XWalkComputerVisionColor color)
{
    static_cast<TestState*>(context)->color = color;
}

/** @brief Records the face-detector state. */
void setFace(agent::contextpointer context, agent::boolean enabled)
{
    static_cast<TestState*>(context)->faceEnabled = enabled;
}

/** @brief Records the QR-detector state. */
void setQr(agent::contextpointer context, agent::boolean enabled)
{
    static_cast<TestState*>(context)->qrEnabled = enabled;
}

/** @brief Returns one deterministic observation. */
xwalk::agent::XWalkComputerVisionObservation observe(
    agent::contextpointer context)
{
    const TestState& state = *static_cast<TestState*>(context);
    xwalk::agent::XWalkComputerVisionObservation observation;
    if (state.color != xwalk::agent::XWalkComputerVisionColor::Close)
    {
        observation.color = {1U, 120, 80, 40U, 30U};
    }
    if (state.faceEnabled)
    {
        observation.face = {1U, 300, 200, 100U, 120U};
    }
    if (state.qrEnabled)
    {
        observation.qrData = "xwalk-qr";
    }
    return observation;
}

/** @brief Accumulates simulated delay without sleeping. */
void delay(agent::contextpointer context, agent::uint32 durationMs)
{
    static_cast<TestState*>(context)->delayTotalMs += durationMs;
}

/** @brief Applies one deterministic cancellation limit. */
agent::boolean continueOperation(agent::contextpointer context)
{
    TestState& state = *static_cast<TestState*>(context);
    const agent::boolean result = state.continueCount < state.continueLimit;
    ++state.continueCount;
    return result;
}

/** @brief Returns the complete deterministic callback table. */
xwalk::agent::XWalkComputerVisionCallbacks callbacks()
{
    return {&start, &stop, &capture, &setColor, &setFace, &setQr,
        &observe, &delay, &continueOperation};
}

/** @brief Exercises all source-compatible key groups and retained state. */
void testBehavior()
{
    TestState state;
    xwalk::agent::XWalkComputerVision vision(&state, callbacks());
    const agent::boolean started = vision.start();
    assert(started);
    assert(vision.started());

    xwalk::agent::XWalkComputerVisionResult result = vision.handleKey("1");
    assert(result.event == xwalk::agent::XWalkComputerVisionEvent::ColorChanged);
    assert(result.color == xwalk::agent::XWalkComputerVisionColor::Red);
    assert(state.color == xwalk::agent::XWalkComputerVisionColor::Red);

    result = vision.handleKey("F");
    assert(result.event == xwalk::agent::XWalkComputerVisionEvent::FaceChanged);
    assert(result.faceEnabled);
    assert(state.faceEnabled);

    result = vision.handleKey("r");
    assert(result.event == xwalk::agent::XWalkComputerVisionEvent::QrChanged);
    assert(result.qrEnabled);
    assert(result.qrChanged);
    assert(result.observation.qrData == "xwalk-qr");

    result = vision.handleKey("s");
    assert(result.event == xwalk::agent::XWalkComputerVisionEvent::ObjectsShown);
    assert(result.observation.color.count == 1U);
    assert(result.observation.face.count == 1U);
    assert(!result.qrChanged);

    result = vision.handleKey("q");
    assert(result.event == xwalk::agent::XWalkComputerVisionEvent::PhotoCaptured);
    assert(result.photoPath == "/tmp/photo_2026-08-04-12-00-00.jpg");

    result = vision.handleKey("0");
    assert(result.event == xwalk::agent::XWalkComputerVisionEvent::ColorChanged);
    assert(result.color == xwalk::agent::XWalkComputerVisionColor::Close);

    result = vision.handleKey("r");
    assert(result.event == xwalk::agent::XWalkComputerVisionEvent::QrChanged);
    assert(!result.qrEnabled);
    assert(state.delayTotalMs == 3'500U);

    vision.stop();
    assert(!vision.started());
    assert(state.stopCount == 1U);
}

/** @brief Verifies cancellation and callback validation failures. */
void testValidation()
{
    TestState state;
    xwalk::agent::XWalkComputerVision vision(&state, callbacks());
    xwalk::hal::test::expectFailure([&vision]()
    {
        static_cast<void>(vision.handleKey("q"));
    });
    const agent::boolean started = vision.start();
    assert(started);
    state.continueLimit = state.continueCount;
    const xwalk::agent::XWalkComputerVisionResult result = vision.handleKey("x");
    assert(result.event == xwalk::agent::XWalkComputerVisionEvent::Cancelled);

    xwalk::agent::XWalkComputerVisionCallbacks invalidCallbacks = callbacks();
    invalidCallbacks.observe = nullptr;
    xwalk::hal::test::expectFailure([&state, &invalidCallbacks]()
    {
        xwalk::agent::XWalkComputerVision invalid(&state, invalidCallbacks);
        static_cast<void>(invalid);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/** @brief Runs every computer-vision host-test scenario. @return Zero on success. */
int main()
{
    testBehavior();
    testValidation();
    return 0;
}
