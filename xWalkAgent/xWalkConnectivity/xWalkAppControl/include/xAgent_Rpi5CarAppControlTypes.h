/******************************************************************************
 * @file        xAgent_Rpi5CarAppControlTypes.h
 * @brief       Declares mobile-app control transport and state contracts.
 * @project     xWalk Firmware
 * @module      xWalkAppControl
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_APP_CONTROL_TYPES_H
#define XAGENT_RPI5CAR_APP_CONTROL_TYPES_H

#include "xAgent_Rpi5CarComputerVisionTypes.h"
#include "xHal_Rpi5CarLineTrackerTypes.h"

namespace xwalk::agent
{

/** @brief Stores the latest A-Q values consumed from SunFounder Controller. */
struct XWalkAppControlInput
{
    agent::boolean hornRequested{};
    agent::string spokenCommand{};
    agent::boolean lineTrackingEnabled{};
    agent::boolean obstacleAvoidanceEnabled{};
    agent::boolean driveJoystickAvailable{};
    agent::float64 driveX{};
    agent::float64 driveY{};
    agent::boolean cameraJoystickAvailable{};
    agent::float64 cameraPanDegrees{};
    agent::float64 cameraTiltDegrees{};
    agent::boolean colorDetectionEnabled{};
    agent::boolean faceDetectionEnabled{};
    agent::boolean objectDetectionEnabled{};
};

/** @brief Stores telemetry published to the mobile application. */
struct XWalkAppControlTelemetry
{
    agent::float64 speedPercent{};
    hal::linetrackervalues grayscale{};
    agent::float64 distanceCm{};
    agent::string videoUrl{};
};

/** @brief Identifies the primary action applied during one foreground step. */
enum class XWalkAppControlEvent : agent::uint8
{
    Idle = 0U,
    JoystickMotion,
    VoiceMotion,
    LineTracking,
    ObstacleAvoidance,
    HornRequested,
    ObjectDetectionUnsupported,
    Cancelled
};

/** @brief Reports one bounded mobile-app control iteration. */
struct XWalkAppControlResult
{
    XWalkAppControlEvent event{XWalkAppControlEvent::Idle};
    XWalkAppControlTelemetry telemetry{};
    agent::boolean hornRequested{};
    agent::boolean objectDetectionWarning{};
};

using appcontrolstartcallback = agent::boolean (*)(agent::contextpointer,
    agent::stringview, agent::stringview, agent::uint16);
using appcontrolstopcallback = void (*)(agent::contextpointer) noexcept;
using appcontrolpollcallback = XWalkAppControlInput (*)(agent::contextpointer);
using appcontrolpublishcallback = void (*)(agent::contextpointer,
    const XWalkAppControlTelemetry&);

/** @brief Defines the application-owned SunFounder transport boundary. */
struct XWalkAppControlCallbacks
{
    agent::contextpointer transportContext{nullptr};
    appcontrolstartcallback start{nullptr};
    appcontrolstopcallback stop{nullptr};
    appcontrolpollcallback poll{nullptr};
    appcontrolpublishcallback publish{nullptr};
    agent::contextpointer visionContext{nullptr};
    XWalkComputerVisionCallbacks vision{};
};

/** @brief Stores bounded source-compatible application-control settings. */
struct XWalkAppControlConfiguration
{
    agent::string controllerName{"Picarx-001"};
    agent::string controllerType{"Picarx"};
    agent::string videoUrl{};
    agent::uint16 controllerPort{8'765U};
    agent::float64 lineTrackingSpeedPercent{10.0};
    agent::float64 lineTrackingAngleDegrees{20.0};
    agent::float64 obstacleSpeedPercent{40.0};
    agent::uint32 maximumLineRecoverySamples{1'000U};
    agent::uint32 sampleDelayMs{10U};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_APP_CONTROL_TYPES_H */
