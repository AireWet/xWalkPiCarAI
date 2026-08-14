/******************************************************************************
 * @file        xControllerCameraHandler.cpp
 * @brief       Implements the CameraHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based handler group.
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
 * @brief Executes the camera command.
 * @param[in] request Validated camera axis and angle in degrees.
 * @return Zero after the servo command completes.
 */
::ctrl::int32 XWalkController::XWALK_handlerCamera(const XWalkCameraRequest& request)
{
    if (request.axis == XWalkCameraAxis::Pan)
    {
        picarxObject->setCameraPanAngle(request.angleDegrees);
    }
    else
    {
        picarxObject->setCameraTiltAngle(request.angleDegrees);
    }
    return 0;
}

} /* namespace xwalk::ctrl */
