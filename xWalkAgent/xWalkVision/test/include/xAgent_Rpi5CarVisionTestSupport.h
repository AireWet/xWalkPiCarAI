/******************************************************************************
 * @file        xAgent_Rpi5CarVisionTestSupport.h
 * @brief       Declares reusable deterministic vision callback state.
 * @project     xWalk Firmware
 * @module      xWalkVision Group GoogleTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VISION_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_VISION_TEST_SUPPORT_H

#include "xAgent_Rpi5CarComputerVisionTypes.h"
#include "xAgent_Rpi5CarPicarxTestSupport.h"

#include <limits>

namespace xwalk::agent::test::vision
{

/** @brief Stores deterministic provider behavior and callback observations. */
struct VisionState
{
    XWalkComputerVisionObservation observation{};
    XWalkComputerVisionColor selectedColor{XWalkComputerVisionColor::Close};
    agent::boolean faceEnabled{};
    agent::boolean startResult{true};
    agent::boolean continueResult{true};
    agent::uint32 continueCallsBeforeCancel{
        std::numeric_limits<agent::uint32>::max()};
    agent::uint32 startCount{};
    agent::uint32 stopCount{};
    agent::uint32 delayCount{};
    agent::uint32 totalDelayMs{};
    agent::uint32 continueCount{};
};

/** @brief Owns one initialized simulator-backed vehicle and isolated config. */
struct VisionVehicleRig
{
    agent::filesystempath configurationPath;
    agent::test::picarx::SimulationRig simulation;

    VisionVehicleRig();
    ~VisionVehicleRig();
    VisionVehicleRig(const VisionVehicleRig&) = delete;
    VisionVehicleRig& operator=(const VisionVehicleRig&) = delete;
    VisionVehicleRig(VisionVehicleRig&&) = delete;
    VisionVehicleRig& operator=(VisionVehicleRig&&) = delete;
};

/** @brief Returns a complete callback table backed by one VisionState. */
XWalkComputerVisionCallbacks callbacks() noexcept;

} /* namespace xwalk::agent::test::vision */

#endif /* XAGENT_RPI5CAR_VISION_TEST_SUPPORT_H */
