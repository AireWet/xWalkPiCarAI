/******************************************************************************
 * @file        xAgent_Rpi5CarMoveExampleLifecycle.cpp
 * @brief       Implements movement-example lifecycle and scheduling.
 * @project     xWalk Firmware
 * @module      xWalkMoveExample
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarMoveExample.h"

#include "xHal_Rpi5CarTrace.h"
#include <cstdio>

namespace xwalk::agent
{

    XWalkMoveExample::XWalkMoveExample(XWalkPicarx& picarx,
                                       agent::contextpointer context,
                                       moveexampledelaycallback delayOperation,
                                       moveexamplecontinuecallback continueOperation)
        : picarxObject(&picarx), callbackContext(context), delayCallback(delayOperation),
          continueCallback(continueOperation)
    {
        if ((delayCallback == nullptr) || (continueCallback == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Move example requires complete callbacks");
        }
    }

    XWalkMoveExample::~XWalkMoveExample()
    {
        stop();
    }

    agent::boolean XWalkMoveExample::wait(agent::uint32 durationMs) const
    {
        constexpr agent::uint32 cancellationIntervalMs{20U};
        agent::uint32 remainingMs = durationMs;
        while (remainingMs > 0U)
        {
            const agent::boolean operationRequested = continueCallback(callbackContext);
            if (operationRequested == false)
            {
                return false;
            }
            const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ? remainingMs : cancellationIntervalMs;
            delayCallback(callbackContext, sliceMs);
            remainingMs -= sliceMs;
        }
        return continueCallback(callbackContext);
    }

    void XWalkMoveExample::stop() noexcept
    {
        static_cast<void>(picarxObject->emergencyStop());
    }

} /* namespace xwalk::agent */
