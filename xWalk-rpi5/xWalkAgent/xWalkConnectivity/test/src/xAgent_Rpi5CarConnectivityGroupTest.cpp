/******************************************************************************
 * @file        xAgent_Rpi5CarConnectivityGroupTest.cpp
 * @brief       Tests every connectivity Agent module through GoogleTest.
 * @project     xWalk Firmware
 * @module      xWalkConnectivity Group GoogleTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarAppControl.h"
#include "xAgent_Rpi5CarAppControlTestSupport.h"
#include "xAgent_Rpi5CarGroupTestSupport.h"

#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{

/** @brief Runs the existing SPI child module test in an isolated process. */
void runModuleTest(const char* moduleDirectory, const char* executableName)
{
    const xwalk::agent::filesystempath binary =
        xwalk::agent::test::childTestExecutable(moduleDirectory, executableName);
    const pid_t childProcess = ::fork();
    ASSERT_GE(childProcess, 0);
    if (childProcess == 0)
    {
        ::execl(binary.c_str(), binary.c_str(), static_cast<char*>(nullptr));
        ::_exit(127);
    }
    int status{};
    ASSERT_EQ(::waitpid(childProcess, &status, 0), childProcess);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
}

TEST(XWalkAgentConnectivityGroup, AppControl)
{
    const xwalk::agent::XWalkAppControlConfiguration configuration;
    EXPECT_EQ(configuration.controllerName, "Picarx-001");
    EXPECT_EQ(configuration.controllerPort, 8'765U);
    EXPECT_EQ(configuration.maximumLineRecoverySamples, 1'000U);
}

TEST(XWalkAgentConnectivityGroup, AppControlLifecycleAndInputModes)
{
    xwalk::agent::test::app_control::VehicleRig rig;
    xwalk::agent::test::app_control::State state;
    xwalk::agent::XWalkAppControl control(*rig.simulation.vehicle,
        xwalk::agent::test::app_control::callbacks(state),
        {"controller", "Picarx", "http://127.0.0.1/stream", 8'765U,
            10.0, 20.0, 40.0, 2U, 1U});
    EXPECT_THROW(static_cast<void>(control.step()), xwalk::agent::logicerror);
    ASSERT_TRUE(control.start());
    EXPECT_TRUE(control.start());

    state.input = {};
    state.input.hornRequested = true;
    EXPECT_EQ(control.step().event, xwalk::agent::XWalkAppControlEvent::HornRequested);

    state.input = {};
    state.input.driveJoystickAvailable = true;
    state.input.driveX = -200.0;
    state.input.driveY = 200.0;
    state.input.cameraJoystickAvailable = true;
    state.input.cameraPanDegrees = 200.0;
    state.input.cameraTiltDegrees = -100.0;
    state.input.colorDetectionEnabled = true;
    state.input.faceDetectionEnabled = true;
    state.input.objectDetectionEnabled = true;
    EXPECT_EQ(control.step().event,
        xwalk::agent::XWalkAppControlEvent::ObjectDetectionUnsupported);
    EXPECT_EQ(state.color, xwalk::agent::XWalkComputerVisionColor::Red);
    EXPECT_TRUE(state.faceEnabled);

    state.input = {};
    state.input.driveJoystickAvailable = true;
    state.input.driveY = -50.0;
    EXPECT_EQ(control.step().event,
        xwalk::agent::XWalkAppControlEvent::ObjectDetectionUnsupported);
    EXPECT_EQ(control.step().event,
        xwalk::agent::XWalkAppControlEvent::JoystickMotion);
    state.input.driveY = 0.0;
    EXPECT_EQ(control.step().event,
        xwalk::agent::XWalkAppControlEvent::JoystickMotion);

    for (const xwalk::agent::cstring command :
        {"forward", "backward", "left", "right", "white", "rice", "stop", "unknown"})
    {
        state.input = {};
        state.input.spokenCommand = command;
        EXPECT_EQ(control.step().event, xwalk::agent::XWalkAppControlEvent::VoiceMotion);
    }

    state.input = {};
    state.input.obstacleAvoidanceEnabled = true;
    EXPECT_EQ(control.step().event,
        xwalk::agent::XWalkAppControlEvent::ObstacleAvoidance);

    state.continueResult = false;
    state.input = {};
    EXPECT_EQ(control.step().event, xwalk::agent::XWalkAppControlEvent::Cancelled);
    control.finish();
    control.finish();
    EXPECT_FALSE(control.started());
    EXPECT_GT(state.publishCount, 0U);
}

TEST(XWalkAgentConnectivityGroup, AppControlStartFailuresAndValidation)
{
    xwalk::agent::test::app_control::VehicleRig rig;
    xwalk::agent::test::app_control::State state;
    state.transportStartResult = false;
    xwalk::agent::XWalkAppControl transportFailure(*rig.simulation.vehicle,
        xwalk::agent::test::app_control::callbacks(state));
    EXPECT_FALSE(transportFailure.start());

    state.transportStartResult = true;
    state.visionStartResult = false;
    xwalk::agent::XWalkAppControl visionFailure(*rig.simulation.vehicle,
        xwalk::agent::test::app_control::callbacks(state));
    EXPECT_FALSE(visionFailure.start());
    EXPECT_GT(state.transportStopCount, 0U);

    xwalk::agent::XWalkAppControlCallbacks incomplete =
        xwalk::agent::test::app_control::callbacks(state);
    incomplete.publish = nullptr;
    EXPECT_THROW(xwalk::agent::XWalkAppControl(
                     *rig.simulation.vehicle, incomplete),
        xwalk::agent::invalidargument);

    xwalk::agent::XWalkAppControlConfiguration invalidText;
    invalidText.controllerName.clear();
    EXPECT_THROW(xwalk::agent::XWalkAppControl(*rig.simulation.vehicle,
                     xwalk::agent::test::app_control::callbacks(state), invalidText),
        xwalk::agent::invalidargument);

    const xwalk::agent::fixedarray<xwalk::agent::XWalkAppControlConfiguration,
        9U> invalid{{
        {"c", "t", {}, 0U, 10.0, 20.0, 40.0, 1'000U, 10U},
        {"c", "t", {}, 1U, -1.0, 20.0, 40.0, 1'000U, 10U},
        {"c", "t", {}, 1U, 101.0, 20.0, 40.0, 1'000U, 10U},
        {"c", "t", {}, 1U, 10.0, -1.0, 40.0, 1'000U, 10U},
        {"c", "t", {}, 1U, 10.0, 31.0, 40.0, 1'000U, 10U},
        {"c", "t", {}, 1U, 10.0, 20.0, -1.0, 1'000U, 10U},
        {"c", "t", {}, 1U, 10.0, 20.0, 101.0, 1'000U, 10U},
        {"c", "t", {}, 1U, 10.0, 20.0, 40.0, 0U, 10U},
        {"c", "t", {}, 1U, 10.0, 20.0, 40.0, 1'000U, 0U}
    }};
    for (const auto& configuration : invalid)
    {
        EXPECT_THROW(xwalk::agent::XWalkAppControl(*rig.simulation.vehicle,
                         xwalk::agent::test::app_control::callbacks(state), configuration),
            xwalk::agent::outofrange);
    }
}

TEST(XWalkAgentConnectivityGroup, SpiTransfer)
{
    runModuleTest("xWalkSpiTransfer", "xWalkSpiTransferTest");
}

} /* namespace */
