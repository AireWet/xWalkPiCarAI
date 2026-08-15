/******************************************************************************
 * @file        xAgent_Rpi5CarServoMotorCalibrationLifecycle.cpp
 * @brief       Implements servo/motor calibration lifecycle management.
 *
 * @project     xWalk Firmware
 * @module      xWalkServoMotorCalibration
 *
 * @author      Joxy John
 * @date        2026-08-04
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

#include "xAgent_Rpi5CarServoMotorCalibration.h"

#include "xHal_Rpi5CarTrace.h"
#include <cstdio>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /**
     * @brief Binds one PiCar-X coordinator and injected scheduling operations.
     * @param[in] picarx PiCar-X coordinator that must outlive this object.
     * @param[in,out] context Optional callback context that must outlive this
     * object.
     * @param[in] delayOperation Non-null synchronous delay operation.
     * @param[in] continueOperation Non-null synchronous cancellation query.
     * @throws std::invalid_argument If either callback is null.
     */
    XWalkServoMotorCalibration::XWalkServoMotorCalibration(XWalkPicarx& picarx,
                                                           agent::contextpointer context,
                                                           servomotorcalibrationdelaycallback delayOperation,
                                                           servomotorcalibrationcontinuecallback continueOperation)
        : picarxObject(&picarx), callbackContext(context), delayCallback(delayOperation),
          continueCallback(continueOperation)
    {
        if ((delayCallback == nullptr) || (continueCallback == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Servo/motor calibration requires complete callbacks");
        }
        resultValue.servoOffsets = {picarx.directionServoCalibration(),
                                    picarx.cameraPanServoCalibration(),
                                    picarx.cameraTiltServoCalibration()};
        resultValue.motorDirections = picarx.motorDirections();
    }

    /** @brief Performs a best-effort drive-motor stop. */
    XWalkServoMotorCalibration::~XWalkServoMotorCalibration()
    {
        stop();
    }

    /**
     * @brief Waits in cancellable slices no longer than 20 milliseconds.
     * @param[in] durationMs Requested delay in milliseconds.
     * @return `true` when execution may continue after the delay.
     */
    agent::boolean XWalkServoMotorCalibration::wait(agent::uint32 durationMs) const
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

    /** @brief Latches non-throwing emergency motor shutdown. */
    void XWalkServoMotorCalibration::stop() noexcept
    {
        static_cast<void>(picarxObject->emergencyStop());
    }

} /* namespace xwalk::agent */
