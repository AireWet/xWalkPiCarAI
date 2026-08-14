/******************************************************************************
 * @file        xAgent_Rpi5CarVisionGroupTest.cpp
 * @brief       Tests every vision Agent module through GoogleTest.
 * @details     Runs existing child tests and verifies newer public contracts.
 * @project     xWalk Firmware
 * @module      xWalkVision Group GoogleTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBullFight.h"
#include "xAgent_Rpi5CarFaceTracking.h"
#include "xAgent_Rpi5CarGroupTestSupport.h"
#include "xAgent_Rpi5CarRoadUserSafety.h"
#include "xAgent_Rpi5CarMjpegStream.h"
#include "xAgent_Rpi5CarTreasureHunt.h"
#include "xAgent_Rpi5CarVideoCar.h"
#include "xAgent_Rpi5CarVisionTestSupport.h"

#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{

/** @brief Runs one no-argument child module test in an isolated process. */
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

TEST(XWalkAgentVisionGroup, ComputerVision)
{
    runModuleTest("xWalkComputerVision", "xWalkComputerVisionTest");
}

TEST(XWalkAgentVisionGroup, FaceTracking)
{
    const xwalk::agent::XWalkFaceTrackingConfiguration configuration;
    EXPECT_EQ(configuration.frameWidthPixels, 640U);
    EXPECT_EQ(configuration.frameHeightPixels, 480U);
    EXPECT_DOUBLE_EQ(configuration.maximumAngleDegrees, 35.0);
}

TEST(XWalkAgentVisionGroup, FaceTrackingLifecycleAndCancellation)
{
    xwalk::agent::test::vision::VisionVehicleRig rig;
    xwalk::agent::test::vision::VisionState state;
    state.observation.face = {1U, 640, 0, 40U, 40U};
    xwalk::agent::XWalkFaceTracking tracker(*rig.simulation.vehicle, &state,
        xwalk::agent::test::vision::callbacks());
    EXPECT_THROW(static_cast<void>(tracker.step()), xwalk::agent::logicerror);
    ASSERT_TRUE(tracker.start());
    EXPECT_TRUE(tracker.start());
    EXPECT_TRUE(state.faceEnabled);
    EXPECT_EQ(tracker.step().state, xwalk::agent::XWalkFaceTrackingState::Tracking);
    state.observation.face = {1U, 0, 480, 40U, 40U};
    EXPECT_EQ(tracker.step().state, xwalk::agent::XWalkFaceTrackingState::Tracking);
    state.observation.face = {};
    EXPECT_EQ(tracker.step().state, xwalk::agent::XWalkFaceTrackingState::Searching);
    state.continueResult = false;
    EXPECT_EQ(tracker.step().state, xwalk::agent::XWalkFaceTrackingState::Cancelled);
    tracker.finish();
    tracker.finish();
    EXPECT_FALSE(tracker.started());
}

TEST(XWalkAgentVisionGroup, FaceTrackingRejectsInvalidDependenciesAndBounds)
{
    xwalk::agent::test::vision::VisionVehicleRig rig;
    xwalk::agent::test::vision::VisionState state;
    xwalk::agent::XWalkComputerVisionCallbacks incomplete =
        xwalk::agent::test::vision::callbacks();
    incomplete.start = nullptr;
    EXPECT_THROW(xwalk::agent::XWalkFaceTracking(
                     *rig.simulation.vehicle, &state, incomplete),
        xwalk::agent::invalidargument);
    const xwalk::agent::fixedarray<xwalk::agent::XWalkFaceTrackingConfiguration,
        10U> invalid{{
        {15U, 480U, 10.0, 35.0, 50U, 100U},
        {7'681U, 480U, 10.0, 35.0, 50U, 100U},
        {640U, 15U, 10.0, 35.0, 50U, 100U},
        {640U, 4'321U, 10.0, 35.0, 50U, 100U},
        {640U, 480U, 0.0, 35.0, 50U, 100U},
        {640U, 480U, 181.0, 35.0, 50U, 100U},
        {640U, 480U, 10.0, 0.0, 50U, 100U},
        {640U, 480U, 10.0, 91.0, 50U, 100U},
        {640U, 480U, 10.0, 35.0, 0U, 100U},
        {640U, 480U, 10.0, 35.0, 50U, 1'001U}
    }};
    for (const auto& configuration : invalid)
    {
        EXPECT_THROW(xwalk::agent::XWalkFaceTracking(*rig.simulation.vehicle,
                         &state, xwalk::agent::test::vision::callbacks(), configuration),
            xwalk::agent::outofrange);
    }
}

TEST(XWalkAgentVisionGroup, BullFight)
{
    const xwalk::agent::XWalkBullFightConfiguration configuration;
    EXPECT_DOUBLE_EQ(configuration.speedPercent, 50.0);
    EXPECT_EQ(configuration.sampleDelayMs, 50U);
}

TEST(XWalkAgentVisionGroup, BullFightLifecycleAndCancellation)
{
    xwalk::agent::test::vision::VisionVehicleRig rig;
    xwalk::agent::test::vision::VisionState state;
    state.observation.color = {1U, 640, 0, 40U, 40U};
    xwalk::agent::XWalkBullFight bullFight(*rig.simulation.vehicle, &state,
        xwalk::agent::test::vision::callbacks());
    EXPECT_THROW(static_cast<void>(bullFight.step()), xwalk::agent::logicerror);
    ASSERT_TRUE(bullFight.start());
    EXPECT_TRUE(bullFight.start());
    EXPECT_EQ(state.selectedColor, xwalk::agent::XWalkComputerVisionColor::Red);
    EXPECT_EQ(bullFight.step().state, xwalk::agent::XWalkBullFightState::Pursuing);
    state.observation.color = {1U, 0, 480, 40U, 40U};
    EXPECT_EQ(bullFight.step().state, xwalk::agent::XWalkBullFightState::Pursuing);
    state.observation.color = {};
    EXPECT_EQ(bullFight.step().state, xwalk::agent::XWalkBullFightState::Searching);
    state.continueResult = false;
    EXPECT_EQ(bullFight.step().state, xwalk::agent::XWalkBullFightState::Cancelled);
    bullFight.finish();
    bullFight.finish();
    EXPECT_FALSE(bullFight.started());
}

TEST(XWalkAgentVisionGroup, BullFightRejectsInvalidDependenciesAndBounds)
{
    xwalk::agent::test::vision::VisionVehicleRig rig;
    xwalk::agent::test::vision::VisionState state;
    xwalk::agent::XWalkComputerVisionCallbacks incomplete =
        xwalk::agent::test::vision::callbacks();
    incomplete.observe = nullptr;
    EXPECT_THROW(xwalk::agent::XWalkBullFight(
                     *rig.simulation.vehicle, &state, incomplete),
        xwalk::agent::invalidargument);
    const xwalk::agent::fixedarray<xwalk::agent::XWalkBullFightConfiguration,
        12U> invalid{{
        {15U, 480U, 10.0, 35.0, 50.0, 50U, 100U},
        {7'681U, 480U, 10.0, 35.0, 50.0, 50U, 100U},
        {640U, 15U, 10.0, 35.0, 50.0, 50U, 100U},
        {640U, 4'321U, 10.0, 35.0, 50.0, 50U, 100U},
        {640U, 480U, 0.0, 35.0, 50.0, 50U, 100U},
        {640U, 480U, 181.0, 35.0, 50.0, 50U, 100U},
        {640U, 480U, 10.0, 0.0, 50.0, 50U, 100U},
        {640U, 480U, 10.0, 91.0, 50.0, 50U, 100U},
        {640U, 480U, 10.0, 35.0, -1.0, 50U, 100U},
        {640U, 480U, 10.0, 35.0, 101.0, 50U, 100U},
        {640U, 480U, 10.0, 35.0, 50.0, 0U, 100U},
        {640U, 480U, 10.0, 35.0, 50.0, 50U, 1'001U}
    }};
    for (const auto& configuration : invalid)
    {
        EXPECT_THROW(xwalk::agent::XWalkBullFight(*rig.simulation.vehicle,
                         &state, xwalk::agent::test::vision::callbacks(), configuration),
            xwalk::agent::outofrange);
    }
}

TEST(XWalkAgentVisionGroup, TreasureHunt)
{
    EXPECT_EQ(xwalk::agent::XWalkTreasureHunt::colorName(
        xwalk::agent::XWalkComputerVisionColor::Purple), "purple");
    const xwalk::agent::XWalkTreasureHuntConfiguration configuration;
    EXPECT_EQ(configuration.movementDelayMs, 500U);
}

TEST(XWalkAgentVisionGroup, VideoRecording)
{
    runModuleTest("xWalkVideoRecording", "xWalkVideoRecordingTest");
}

TEST(XWalkAgentVisionGroup, VideoCar)
{
    EXPECT_EQ(xwalk::agent::XWalkVideoCar::motionName(
        xwalk::agent::XWalkVideoCarMotion::Forward), "forward");
    const xwalk::agent::XWalkVideoCarConfiguration configuration;
    EXPECT_EQ(configuration.maximumSpeedPercent, 100U);
}

TEST(XWalkAgentVisionGroup, VideoCarHandlesEveryKeyAndLifecycleState)
{
    xwalk::agent::test::vision::VisionVehicleRig rig;
    xwalk::agent::test::vision::VisionState state;
    xwalk::agent::XWalkVideoCar videoCar(*rig.simulation.vehicle, &state,
        xwalk::agent::test::vision::callbacks(), {10U, 30U, 20U, 30.0, 0U, 1U});
    EXPECT_THROW(static_cast<void>(videoCar.handleKey("w")),
        xwalk::agent::logicerror);
    ASSERT_TRUE(videoCar.start());
    EXPECT_TRUE(videoCar.start());
    EXPECT_EQ(videoCar.handleKey("w").motion,
        xwalk::agent::XWalkVideoCarMotion::Forward);
    EXPECT_EQ(videoCar.handleKey("o").speedPercent, 20U);
    EXPECT_EQ(videoCar.handleKey("o").speedPercent, 30U);
    EXPECT_EQ(videoCar.handleKey("o").speedPercent, 30U);
    EXPECT_EQ(videoCar.handleKey("s").motion,
        xwalk::agent::XWalkVideoCarMotion::Backward);
    EXPECT_EQ(videoCar.handleKey("a").motion,
        xwalk::agent::XWalkVideoCarMotion::TurnLeft);
    EXPECT_EQ(videoCar.handleKey("D").motion,
        xwalk::agent::XWalkVideoCarMotion::TurnRight);
    EXPECT_EQ(videoCar.handleKey("t").event,
        xwalk::agent::XWalkVideoCarEvent::PhotoCaptured);
    EXPECT_EQ(videoCar.handleKey("ignored").event,
        xwalk::agent::XWalkVideoCarEvent::Ignored);
    EXPECT_EQ(videoCar.handleKey("f").motion,
        xwalk::agent::XWalkVideoCarMotion::Stop);
    EXPECT_EQ(videoCar.handleKey("p").speedPercent, 10U);
    EXPECT_EQ(videoCar.handleKey("p").speedPercent, 0U);
    state.continueResult = false;
    EXPECT_EQ(videoCar.handleKey("w").event,
        xwalk::agent::XWalkVideoCarEvent::Cancelled);
    videoCar.finish();
    videoCar.finish();
    EXPECT_FALSE(videoCar.started());
    EXPECT_EQ(xwalk::agent::XWalkVideoCar::motionName(
                  xwalk::agent::XWalkVideoCarMotion::Backward), "backward");
    EXPECT_EQ(xwalk::agent::XWalkVideoCar::motionName(
                  xwalk::agent::XWalkVideoCarMotion::TurnLeft), "turn left");
    EXPECT_EQ(xwalk::agent::XWalkVideoCar::motionName(
                  xwalk::agent::XWalkVideoCarMotion::TurnRight), "turn right");
    EXPECT_EQ(xwalk::agent::XWalkVideoCar::motionName(
                  xwalk::agent::XWalkVideoCarMotion::Stop), "stop");
}

TEST(XWalkAgentVisionGroup, VideoCarRejectsInvalidDependenciesAndBounds)
{
    xwalk::agent::test::vision::VisionVehicleRig rig;
    xwalk::agent::test::vision::VisionState state;
    xwalk::agent::XWalkComputerVisionCallbacks incomplete =
        xwalk::agent::test::vision::callbacks();
    incomplete.capture = nullptr;
    EXPECT_THROW(xwalk::agent::XWalkVideoCar(
                     *rig.simulation.vehicle, &state, incomplete),
        xwalk::agent::invalidargument);
    const xwalk::agent::fixedarray<xwalk::agent::XWalkVideoCarConfiguration,
        8U> invalid{{
        {0U, 100U, 60U, 30.0, 2'000U, 100U},
        {101U, 100U, 60U, 30.0, 2'000U, 100U},
        {10U, 101U, 60U, 30.0, 2'000U, 100U},
        {10U, 100U, 101U, 30.0, 2'000U, 100U},
        {10U, 100U, 60U, 0.0, 2'000U, 100U},
        {10U, 100U, 60U, 46.0, 2'000U, 100U},
        {10U, 100U, 60U, 30.0, 10'001U, 100U},
        {10U, 100U, 60U, 30.0, 2'000U, 0U}
    }};
    for (const auto& configuration : invalid)
    {
        EXPECT_THROW(xwalk::agent::XWalkVideoCar(*rig.simulation.vehicle,
                         &state, xwalk::agent::test::vision::callbacks(), configuration),
            xwalk::agent::outofrange);
    }
    state.startResult = false;
    xwalk::agent::XWalkVideoCar startFailure(*rig.simulation.vehicle, &state,
        xwalk::agent::test::vision::callbacks(), {10U, 100U, 60U, 30.0, 0U, 1U});
    EXPECT_FALSE(startFailure.start());
}

TEST(XWalkAgentVisionGroup, CameraCapture)
{
    runModuleTest("xWalkCameraCapture", "xWalkCameraCaptureTest");
}

TEST(XWalkAgentVisionGroup, RoadUserSafety)
{
    runModuleTest("xWalkRoadUserSafety", "xWalkRoadUserSafetyTest");
}

TEST(XWalkAgentVisionGroup, VideoStreaming)
{
    runModuleTest("xWalkVideoStreaming", "xWalkVideoStreamingTest");
}

} /* namespace */
