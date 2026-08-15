/******************************************************************************
 * @file        xAgent_Rpi5CarAppControlTestSupport.cpp
 * @brief       Implements deterministic AppControl callbacks.
 * @project     xWalk Firmware
 * @module      xWalkAppControlTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarAppControlTestSupport.h"

#include <unistd.h>

namespace xwalk::agent::test::app_control
{

    namespace
    {

        State& state(agent::contextpointer context) noexcept
        {
            return *static_cast<State*>(context);
        }

        agent::boolean
        transportStart(agent::contextpointer context, agent::stringview, agent::stringview, agent::uint16)
        {
            State& value = state(context);
            ++value.transportStartCount;
            return value.transportStartResult;
        }

        void transportStop(agent::contextpointer context) noexcept
        {
            ++state(context).transportStopCount;
        }

        XWalkAppControlInput poll(agent::contextpointer context)
        {
            return state(context).input;
        }

        void publish(agent::contextpointer context, const XWalkAppControlTelemetry& telemetry)
        {
            State& value = state(context);
            value.telemetry = telemetry;
            ++value.publishCount;
        }

        agent::boolean visionStart(agent::contextpointer context)
        {
            State& value = state(context);
            ++value.visionStartCount;
            return value.visionStartResult;
        }

        void visionStop(agent::contextpointer context) noexcept
        {
            ++state(context).visionStopCount;
        }

        void setColor(agent::contextpointer context, XWalkComputerVisionColor color)
        {
            state(context).color = color;
        }

        void setFace(agent::contextpointer context, agent::boolean enabled)
        {
            state(context).faceEnabled = enabled;
        }

        void delay(agent::contextpointer context, agent::uint32)
        {
            ++state(context).delayCount;
        }

        agent::boolean continueOperation(agent::contextpointer context)
        {
            State& value = state(context);
            ++value.continueCount;
            return value.continueResult && (value.continueCount <= value.continueCallsBeforeCancel);
        }

    } /* namespace */

    VehicleRig::VehicleRig()
        : configurationPath(agent::filesystempath("/tmp") /
                            ("xwalk-app-control-" + std::to_string(static_cast<unsigned long>(::getpid())) + ".conf")),
          simulation(configurationPath.string(), false)
    {
        static_cast<void>(simulation.vehicle->initialize());
    }

    VehicleRig::~VehicleRig()
    {
        simulation.vehicle.reset();
        agent::errorcode error;
        static_cast<void>(std::filesystem::remove(configurationPath, error));
        agent::filesystempath temporary = configurationPath;
        temporary += ".tmp";
        static_cast<void>(std::filesystem::remove(temporary, error));
    }

    XWalkAppControlCallbacks callbacks(State& value) noexcept
    {
        XWalkAppControlCallbacks result;
        result.transportContext = &value;
        result.start = &transportStart;
        result.stop = &transportStop;
        result.poll = &poll;
        result.publish = &publish;
        result.visionContext = &value;
        result.vision.start = &visionStart;
        result.vision.stop = &visionStop;
        result.vision.setColor = &setColor;
        result.vision.setFace = &setFace;
        result.vision.delay = &delay;
        result.vision.continueOperation = &continueOperation;
        return result;
    }

} /* namespace xwalk::agent::test::app_control */
