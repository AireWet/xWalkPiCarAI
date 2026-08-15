/******************************************************************************
 * @file        xControllerSelfDriveHandler.cpp
 * @brief       Implements the SelfDriveHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based
 *handler group.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xController.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

    /******************************************************************************
     * Member function definitions
     ******************************************************************************/

    /**
     * @brief Executes one named self-drive preset action.
     * @param[in] request Canonical space-separated action text.
     * @return Zero after a supported action completes; three when the coordinator
     * is unavailable.
     */
    ::ctrl::int32 XWalkController::XWALK_handlerSelfDrive(const XWalkSelfDriveRequest& request)
    {
        if (selfDriveObject == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Self-drive backend unavailable");
            return 3;
        }
        const ::ctrl::boolean operationRequested = operationMayContinue();
        if (operationRequested == false)
        {
            return 0;
        }
        const ::ctrl::boolean actionQueued = selfDriveObject->doAction(request.action);
        if (actionQueued == false)
        {
            XWALK_CTRL_ERROR(XWALK_INVAL, "self-drive action is not supported");
        }
        return 0;
    }

} /* namespace xwalk::ctrl */
