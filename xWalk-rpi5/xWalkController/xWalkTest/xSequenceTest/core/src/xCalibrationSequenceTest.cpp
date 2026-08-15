/******************************************************************************
 * @file        xCalibrationSequenceTest.cpp
 * @brief       Verifies one public CLI command sequence through simulated HAL.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <cassert>

namespace
{
    void testCalibration(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->inputLines = {
            "skip", "skip", "skip", "skip", "10", "raised", "y", "y", "y", "y", "y", "ready", "q", "e", "y"};
        xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
        assert(sequence.run({{"calibrate"}}) == 0);

        assert(context.configuration->get("picarx_motor_speed_calibration") == "10.000000");
        assert(context.configuration->get("line_reference") == "[1000,1000,1000]");
        assert(context.motors->left().speed() == 0.0);
        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {"controller.input",
                                                          "hal.i2c.write",
                                                          "controller.input",
                                                          "controller.continue",
                                                          "hal.i2c.write",
                                                          "controller.delay",
                                                          "hal.i2c.write",
                                                          "controller.input",
                                                          "hal.i2c.read"}));

        context.state->inputLines = {"ready", "q", "e", "y"};
        context.state->inputIndex = 0U;
        context.state->eventLog.clear();
        assert(sequence.run({{"calibrate", "grayscale"}}) == 0);

        assert(context.configuration->get("cliff_reference") == "[1000,1000,1000]");
        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {
                                                             "hal.i2c.write",
                                                             "controller.input",
                                                             "hal.i2c.write",
                                                             "controller.delay",
                                                             "controller.input",
                                                             "hal.i2c.write",
                                                             "hal.i2c.read",
                                                             "controller.delay",
                                                             "controller.input",
                                                             "hal.i2c.write",
                                                             "hal.i2c.read",
                                                             "controller.input",
                                                         }));

        context.state->inputLines = {
            "skip", "5", "y", "-2", "y", "3", "y", "10", "1", "-1", "raised", "y", "y", "y", "y", "y"};
        context.state->inputIndex = 0U;
        context.state->eventLog.clear();
        assert(sequence.run({{"calibrate", "servo-motor"}}) == 0);

        assert(context.configuration->get("picarx_dir_servo") == "5.000000");
        assert(context.configuration->get("picarx_cam_pan_servo") == "-2.000000");
        assert(context.configuration->get("picarx_cam_tilt_servo") == "3.000000");
        assert(context.configuration->get("picarx_dir_motor") == "[1,-1]");
        assert(context.configuration->get("picarx_calibration_verified") == "true");
        assert(context.motors->left().speed() == 0.0);
        assert(context.motors->right().speed() == 0.0);
        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {"controller.input",
                                                          "hal.i2c.write",
                                                          "controller.input",
                                                          "hal.i2c.write",
                                                          "controller.input",
                                                          "hal.i2c.write",
                                                          "controller.input",
                                                          "controller.input",
                                                          "controller.input",
                                                          "hal.i2c.write",
                                                          "controller.input",
                                                          "controller.continue",
                                                          "hal.i2c.write",
                                                          "controller.delay",
                                                          "hal.i2c.write"}));
    }
} // namespace

/** @brief Runs the calibration controller-to-HAL host sequence. @return Zero on success. */
int xWalkCalibrationCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testCalibration);
}
