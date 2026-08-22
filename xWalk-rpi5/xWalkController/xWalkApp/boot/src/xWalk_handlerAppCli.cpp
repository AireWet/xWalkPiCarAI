/******************************************************************************
 * @file        xWalk_handlerAppCli.cpp
 * @brief       Implements scheduler-backed CLI signal handling.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xWalk_handlerAppCli.h"

#include "xControllerApplicationSupport.h"
#include "xControllerSchedulerMacros.h"
#include "xControllerSchedulerRuntimeMacros.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::ctrl
{

    /** @brief Starts the CLI runner without claiming a backend resource. */
    static ::ctrl::int32 startRunner(::ctrl::contextpointer context) noexcept
    {
        static_cast<void>(context);
        return 0;
    }

    /** @brief Stops the CLI runner after the synchronous command has returned. */
    static ::ctrl::int32 stopRunner(::ctrl::contextpointer context) noexcept
    {
        static_cast<void>(context);
        return 0;
    }

    ::ctrl::int32 xWalk_handlerAppCli(::ctrl::uint32 signalNumber,
                                      ::ctrl::uint32 moduleType,
                                      xWalkSignalHandler_LPP signalCallback,
                                      ::ctrl::contextpointer context) noexcept
    {
        if ((signalNumber == 0U) || (signalCallback == nullptr) || (context == nullptr))
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Scheduled Controller command arguments are invalid");
            return 2;
        }

        const xModuleCallbacks callbacks{&startRunner, &stopRunner, nullptr, nullptr, context, signalCallback};
        const ::ctrl::boolean signalHandlingActivated = XWALK_activateOperationSignalHandling();
        if (signalHandlingActivated == false)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Parent could not activate scheduler cancellation handling");
            return 2;
        }
        const ::ctrl::int32 openStatus = CXX_openCtrl_LPP(moduleType, &callbacks, &XWALK_continueOperation, nullptr);
        if (openStatus != XWALK_SCHEDULER_OK)
        {
            return 2;
        }

        const xWalkEncodedPayload payload{nullptr, 0U};
        const ::ctrl::int32 sendStatus = cxx_xWalkCtrlSend_LPP(&payload, signalNumber);
        if (sendStatus != XWALK_SCHEDULER_OK)
        {
            CXX_closeCtrl_LPP();
            return 2;
        }

        ::ctrl::int32 commandResult{2};
        const ::ctrl::int32 waitStatus = CXX_waitCtrl_LPP(signalNumber, &commandResult);
        if (waitStatus != XWALK_SCHEDULER_OK)
        {
            commandResult = 2;
        }
        CXX_closeCtrl_LPP();
        return commandResult;
    }

} /* namespace xwalk::ctrl */
