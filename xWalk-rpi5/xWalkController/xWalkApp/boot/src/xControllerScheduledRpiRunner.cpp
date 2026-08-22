/******************************************************************************
 * @file        xControllerScheduledRpiRunner.cpp
 * @brief       Implements scheduler-owned Raspberry Pi Controller boot.
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

#include "xControllerScheduledRunner.h"
#include "xWalk_handlerAppCli.h"

#include "xControllerApplicationSupport.h"
#include "xControllerBootMode.h"
#include "xControllerCommands.h"
#include "xControllerRunner.h"

#include "xAgent_Rpi5CarBootRpi.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::ctrl
{

    /** @brief Constructs all selected hardware resources inside the scheduler child. */
    static ::ctrl::int32 rpiCmdCb(::ctrl::contextpointer context, const XWalkSignal* signal)
    {
        if (signal == nullptr)
        {
            return 2;
        }
        auto* bootContext = static_cast<XWalkRunArgs*>(context);
        const ::ctrl::boolean bootContextInvalid = (bootContext == nullptr) ||
                                                   (bootContext->commandArguments == nullptr) ||
                                                   bootContext->configurationFilePath.empty();
        if (bootContextInvalid)
        {
            return 2;
        }
        agent::XWalkBootRpi boot(XWALK_selectBootMode(*bootContext->commandArguments),
                                 bootContext->configurationFilePath);
        const agent::xAgentContext agentContext{bootContext, &XWALK_runController};
        return XWALK_reply(signal, boot.run(agentContext));
    }

    ::ctrl::int32 XWALK_runRpi(XWalkRunArgs* runArgs) noexcept
    {
        if ((runArgs == nullptr) || (runArgs->commandArguments == nullptr))
        {
            return 2;
        }
        const XWalkControllerCommandRequest commandRequest = XWALK_parseControllerCommand(*runArgs->commandArguments);
        const ::ctrl::uint32 moduleType = XWALK_selectBootMode(*runArgs->commandArguments);
        return xWalk_handlerAppCli(commandRequest.command, moduleType, &rpiCmdCb, runArgs);
    }

} /* namespace xwalk::ctrl */
