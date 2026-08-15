/******************************************************************************
 * @file        xControllerCalibrationHandler.cpp
 * @brief       Implements the CalibrationHandler command responsibility.
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
     * @brief Executes full, grayscale-only, or servo-and-motor calibration.
     * @param[in] request Validated calibration workflow selection.
     * @return Zero after stationary sampling or verified actuator calibration;
     * otherwise two.
     */
    ::ctrl::int32 XWalkController::XWALK_handlerCalibration(const XWalkCalibrationRequest& request)
    {
        if (request.mode == XWalkCalibrationMode::Grayscale)
        {
            XWALK_CTRL_TRACE_UID0(CTRL .004, "=== PiCar-X Grayscale Calibration ===");
            const ::ctrl::boolean calibrated = calibrateGrayscaleReferences();
            XWALK_CTRL_TRACE_UID1(
                CTRL .086, "%s", calibrated ? "Grayscale calibration complete!" : "Grayscale calibration incomplete");
            return calibrated ? 0 : 2;
        }
        if (request.mode == XWalkCalibrationMode::ServoMotor)
        {
            XWALK_CTRL_TRACE_UID0(CTRL .005, "=== PiCar-X Servo and Motor Calibration ===");
            XWALK_CTRL_TRACE_UID0(CTRL .006, "Raise the wheels before accepting any motor verification prompt.");
            const ::ctrl::boolean verified = calibrateServoMotor(true);
            XWALK_CTRL_TRACE_UID1(CTRL .087,
                                  "%s",
                                  verified ? "Servo and motor calibration complete!"
                                           : "Servo and motor calibration incomplete");
            return verified ? 0 : 2;
        }
        XWALK_CTRL_TRACE_UID0(CTRL .007, "=== PiCar-X Servo Calibration ===");
        XWALK_CTRL_TRACE_UID0(CTRL .008, "This will help you calibrate the steering servo and camera gimbal.");
        const ::ctrl::boolean actuatorsVerified = calibrateServoMotor(false);
        const ::ctrl::boolean grayscaleCalibrated = calibrateGrayscaleReferences();
        const ::ctrl::boolean completed = actuatorsVerified && grayscaleCalibrated;
        XWALK_CTRL_TRACE_UID1(CTRL .088, "%s", completed ? "Calibration complete!" : "Calibration incomplete");
        return completed ? 0 : 2;
    }

} /* namespace xwalk::ctrl */
