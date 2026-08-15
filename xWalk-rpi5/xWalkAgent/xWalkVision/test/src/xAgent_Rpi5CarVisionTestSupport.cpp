/******************************************************************************
 * @file        xAgent_Rpi5CarVisionTestSupport.cpp
 * @brief       Implements reusable deterministic vision callbacks.
 * @project     xWalk Firmware
 * @module      xWalkVision Group GoogleTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarVisionTestSupport.h"

#include <unistd.h>

namespace xwalk::agent::test::vision
{

    namespace
    {

        VisionState& state(agent::contextpointer context) noexcept
        {
            return *static_cast<VisionState*>(context);
        }

        agent::boolean start(agent::contextpointer context)
        {
            VisionState& value = state(context);
            ++value.startCount;
            return value.startResult;
        }

        void stop(agent::contextpointer context) noexcept
        {
            ++state(context).stopCount;
        }

        agent::string capture(agent::contextpointer)
        {
            return "/tmp/xwalk-vision-test.jpg";
        }

        void setColor(agent::contextpointer context, XWalkComputerVisionColor color)
        {
            state(context).selectedColor = color;
        }

        void setFace(agent::contextpointer context, agent::boolean enabled)
        {
            state(context).faceEnabled = enabled;
        }

        void setQr(agent::contextpointer, agent::boolean)
        {
        }

        XWalkComputerVisionObservation observe(agent::contextpointer context)
        {
            return state(context).observation;
        }

        void delay(agent::contextpointer context, agent::uint32 durationMs)
        {
            VisionState& value = state(context);
            ++value.delayCount;
            value.totalDelayMs += durationMs;
        }

        agent::boolean continueOperation(agent::contextpointer context)
        {
            VisionState& value = state(context);
            ++value.continueCount;
            return value.continueResult && (value.continueCount <= value.continueCallsBeforeCancel);
        }

    } /* namespace */

    XWalkComputerVisionCallbacks callbacks() noexcept
    {
        return {&start, &stop, &capture, &setColor, &setFace, &setQr, &observe, &delay, &continueOperation};
    }

    VisionVehicleRig::VisionVehicleRig()
        : configurationPath(agent::filesystempath("/tmp") /
                            ("xwalk-vision-group-" + std::to_string(static_cast<unsigned long>(::getpid())) + ".conf")),
          simulation(configurationPath.string(), false)
    {
        static_cast<void>(simulation.vehicle->initialize());
    }

    VisionVehicleRig::~VisionVehicleRig()
    {
        simulation.vehicle.reset();
        agent::errorcode error;
        static_cast<void>(std::filesystem::remove(configurationPath, error));
        agent::filesystempath temporary = configurationPath;
        temporary += ".tmp";
        static_cast<void>(std::filesystem::remove(temporary, error));
    }

} /* namespace xwalk::agent::test::vision */
