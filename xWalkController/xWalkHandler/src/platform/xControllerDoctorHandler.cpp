/******************************************************************************
 * @file        xControllerDoctorHandler.cpp
 * @brief       Implements the DoctorHandler command responsibility.
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

#include "xHal_Rpi5CarExceptions.h"

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
 * @brief Prints one passive hardware preflight report.
 * @param[in] request Validated empty request.
 * @return Zero when every reported check passes; otherwise two.
 */
::ctrl::int32 XWalkController::XWALK_handlerDoctor(const XWalkNoArgumentRequest& request)
{
    static_cast<void>(request);
    if (doctorLinesObject == nullptr)
    {
        output("Doctor backend unavailable");
        return 3;
    }
    ::ctrl::boolean passed = true;
    for (const ::ctrl::string& line : *doctorLinesObject)
    {
        output(line);
        const ::ctrl::boolean lineDifferent =
            static_cast<::ctrl::boolean>(
                line.find("[FAIL]") != ::ctrl::string::npos);
        if (lineDifferent)
        {
            passed = false;
        }
    }
    return passed ? 0 : 2;
}

} /* namespace xwalk::ctrl */
