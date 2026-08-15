/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiBoard.cpp
 * @brief       Implements Robot HAT board selection for Raspberry Pi boot.
 *
 * @details
 * Validates automatic and explicit Robot HAT deployment selections before any
 * actuator resource is claimed.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xHal_Rpi5CarDevice.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent
{

    /**
     * @brief Applies fail-safe automatic or explicit Robot HAT selection.
     * @param[in] detectedInformation Read-only Device Tree discovery result.
     * @param[in] requestedBoard Exact `auto`, `robot_hat_v4`, or `robot_hat_v5`
     * value.
     * @return Validated board information used for hardware composition.
     * @throws std::runtime_error If the requested board cannot be verified safely.
     * @throws std::invalid_argument If the requested board name is unsupported.
     */
    hal::XWalkDeviceInformation XWalkBootRpi::selectBoard(const hal::XWalkDeviceInformation& detectedInformation,
                                                          agent::stringview requestedBoard)
    {
        if (requestedBoard == "auto")
        {
            const agent::boolean boardNotDetected = static_cast<agent::boolean>(detectedInformation.detected == false);
            if (boardNotDetected)
            {
                XWALK_RPIAGENT_ERROR(XWALK_RUNTIME,
                                     "Robot HAT v5 was not detected; select robot_hat_v4 "
                                     "explicitly when applicable");
            }
            return detectedInformation;
        }
        else if (requestedBoard == "robot_hat_v4")
        {
            if (detectedInformation.detected)
            {
                XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, "Configured Robot HAT v4 conflicts with detected Robot HAT v5");
            }
            return {};
        }
        else if (requestedBoard == "robot_hat_v5")
        {
            const agent::boolean boardNotV5 =
                static_cast<agent::boolean>((detectedInformation.detected == false) ||
                                            (detectedInformation.model != hal::XWalkDeviceModel::RobotHatV5));
            if (boardNotV5)
            {
                XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, "Configured Robot HAT v5 was not verified by Device Tree");
            }
            return detectedInformation;
        }
        XWALK_RPIAGENT_ERROR(XWALK_INVAL, "hardware_board must be auto, robot_hat_v4, or robot_hat_v5");
    }

} /* namespace xwalk::agent */
