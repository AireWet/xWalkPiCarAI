/******************************************************************************
 * @file        xComputerVisionSequenceTest.cpp
 * @brief       Verifies every interactive computer-vision CLI key sequence.
 *
 * @details
 * Drives color, face, QR, object-status, photograph, shutdown, and timing
 * behavior through the Controller, Agent, and deterministic provider boundary.
 *
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
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

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <cassert>

namespace
{

/** @brief Verifies every source-compatible computer-vision key in order. */
void testComputerVision(
    xwalk::agent::test::ControllerCommandTestContext& context)
{
    context.state->inputLines = {"1", "f", "r", "s", "q", "0", "r", "x"};
    xwalk::agent::test::XWalkControllerSequence sequence(
        *context.computerVisionController);
    const ctrl::int32 status = sequence.run({{"computer-vision"}});
    assert(status == 0);
    assert(context.state->operationQueries == 190U);
    assert(context.state->delays.size() == 175U);
    assert(context.state->visionCaptureCount == 1U);
    assert(context.state->visionObservationCount == 4U);
    assert(!context.state->visionStarted);
    assert(context.state->visionColor ==
        xwalk::agent::XWalkComputerVisionColor::Close);
    assert(!context.state->visionFaceEnabled);
    assert(!context.state->visionQrEnabled);
    assert(context.state->outputLines.size() == 11U);
    assert(context.state->outputLines[1U] == "Color detect : red");
    assert(context.state->outputLines[2U] == "Face Detect:True");
    assert(context.state->outputLines[3U] == "Waitting for QR code");
    assert(context.state->outputLines[4U] == "QR code:xwalk-qr");
    assert(context.state->outputLines[5U] ==
        "[Color Detect] Coordinate:(120, 80) Size (40, 30)");
    assert(context.state->outputLines[6U] ==
        "[Face Detect] Coordinate:(300, 200) Size (100, 120)");
    assert(context.state->outputLines[7U] ==
        "photo save as /tmp/photo_2026-08-04-12-00-00.jpg");
    assert(context.state->outputLines[8U] == "Color detect : close");
    assert(context.state->outputLines[9U] == "QRcode Detect: close");
    assert(context.state->outputLines[10U] == "Computer vision stopped");
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"vision.start", "controller.input", "vision.color",
            "vision.face", "vision.qr", "vision.observe",
            "vision.observe", "vision.capture", "vision.color",
            "vision.qr", "vision.stop", "controller.output"}));
}

} /* namespace */

/** @brief Runs the computer-vision controller-to-provider host sequence. @return Zero on success. */
int xWalkComputerVisionCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(
        argc, argv, &testComputerVision);
}
