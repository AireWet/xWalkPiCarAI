/******************************************************************************
 * @file        xAgent_Rpi5CarAppControlTestSupport.h
 * @brief       Declares deterministic AppControl callback state.
 * @project     xWalk Firmware
 * @module      xWalkAppControlTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_APP_CONTROL_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_APP_CONTROL_TEST_SUPPORT_H

#include "xAgent_Rpi5CarAppControl.h"
#include "xAgent_Rpi5CarPicarxTestSupport.h"

#include <limits>

namespace xwalk::agent::test::app_control
{

/** @brief Stores injected transport, vision, and scheduling behavior. */
struct State
{
    XWalkAppControlInput input{};
    XWalkAppControlTelemetry telemetry{};
    XWalkComputerVisionColor color{XWalkComputerVisionColor::Close};
    agent::boolean faceEnabled{};
    agent::boolean transportStartResult{true};
    agent::boolean visionStartResult{true};
    agent::boolean continueResult{true};
    agent::uint32 continueCallsBeforeCancel{
        std::numeric_limits<agent::uint32>::max()};
    agent::uint32 transportStartCount{};
    agent::uint32 transportStopCount{};
    agent::uint32 visionStartCount{};
    agent::uint32 visionStopCount{};
    agent::uint32 publishCount{};
    agent::uint32 delayCount{};
    agent::uint32 continueCount{};
};

/** @brief Owns one initialized simulated vehicle and isolated config file. */
struct VehicleRig
{
    agent::filesystempath configurationPath;
    agent::test::picarx::SimulationRig simulation;

    VehicleRig();
    ~VehicleRig();
    VehicleRig(const VehicleRig&) = delete;
    VehicleRig& operator=(const VehicleRig&) = delete;
    VehicleRig(VehicleRig&&) = delete;
    VehicleRig& operator=(VehicleRig&&) = delete;
};

/** @brief Returns complete callbacks backed by one State context. */
XWalkAppControlCallbacks callbacks(State& state) noexcept;

} /* namespace xwalk::agent::test::app_control */

#endif /* XAGENT_RPI5CAR_APP_CONTROL_TEST_SUPPORT_H */
