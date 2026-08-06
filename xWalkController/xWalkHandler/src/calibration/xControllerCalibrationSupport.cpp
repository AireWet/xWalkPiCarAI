/******************************************************************************
 * @file        xControllerCalibrationSupport.cpp
 * @brief       Implements the CalibrationSupport command responsibility.
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

#include "xControllerParsing.h"

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

/** @brief Calibrates actuator offsets and performs raised-wheel verification. */
::ctrl::boolean XWalkController::calibrateServoMotor(
    ::ctrl::boolean configureMotorDirections)
{
    if (servoMotorCalibrationObject == nullptr)
    {
        output("Servo/motor-calibration backend unavailable");
        return false;
    }
    const ::ctrl::boolean servosReset = servoMotorCalibrationObject->resetServos();
    if (servosReset == false)
    {
        return false;
    }
    const ::ctrl::string servoTest = input(
        "Type 'r' to test all servos, or 'skip': ");
    if ((servoTest == "r") || (servoTest == "R"))
    {
        const ::ctrl::boolean servosTested = servoMotorCalibrationObject->testServos();
        if (servosTested == false)
        {
            return false;
        }
    }
    const XWalkServoCalibrationConfig steeringConfig{
        "--- Steering Servo ---",
        "Enter steering offset (-20 to 20, or 'skip'): ",
        -20.0,
        20.0,
        0U};
    const XWalkServoCalibrationConfig cameraPanConfig{
        "--- Camera Pan Servo ---",
        "Enter camera pan offset (-20 to 20, or 'skip'): ",
        -20.0,
        20.0,
        1U};
    const XWalkServoCalibrationConfig cameraTiltConfig{
        "--- Camera Tilt Servo ---",
        "Enter camera tilt offset (-20 to 20, or 'skip'): ",
        -20.0,
        20.0,
        2U};
    const ::ctrl::boolean steeringCalibrated = calibrateServo(steeringConfig);
    if (steeringCalibrated == false)
    {
        return false;
    }
    const ::ctrl::boolean cameraPanCalibrated = calibrateServo(cameraPanConfig);
    if (cameraPanCalibrated == false)
    {
        return false;
    }
    const ::ctrl::boolean cameraTiltCalibrated = calibrateServo(cameraTiltConfig);
    if (cameraTiltCalibrated == false)
    {
        return false;
    }
    const ::ctrl::string motorCorrection = input(
        "Enter motor balance correction (-100 to 100, positive reduces left, or 'skip'): ");
    if ((motorCorrection != "skip") && (motorCorrection != "SKIP"))
    {
        picarxObject->calibrateMotorSpeed(XWALK_parseNumber(
            motorCorrection, "motor balance correction", -100.0, 100.0));
    }
    if (configureMotorDirections)
    {
        calibrateMotorDirection(1U, "left");
        calibrateMotorDirection(2U, "right");
    }
    const ::ctrl::boolean verified = XWALK_handlerFirstRunVerification();
    if (verified == false)
    {
        picarxObject->recordCalibrationVerified(false);
        return false;
    }
    const ::ctrl::string accepted = input(
        "Persist servo offsets and motor directions? (y/n): ");
    if ((accepted != "y") && (accepted != "Y"))
    {
        picarxObject->recordCalibrationVerified(false);
        return false;
    }
    servoMotorCalibrationObject->save();
    picarxObject->recordCalibrationVerified(true);
    return true;
}

/** @brief Optionally persists one explicit motor direction. */
void XWalkController::calibrateMotorDirection(
    ::ctrl::uint8 motorId, ::ctrl::stringview motorName)
{
    const ::ctrl::string direction = input(::ctrl::string("Enter ") + ::ctrl::string(motorName) +
        " motor direction (1, -1, or 'skip'): ");
    if ((direction == "skip") || (direction == "SKIP"))
    {
        return;
    }
    const ::ctrl::float64 parsed = XWALK_parseNumber(
        direction, "motor direction", -1.0, 1.0);
    if ((parsed != -1.0) && (parsed != 1.0))
    {
        XHAL_THROW_INVALID_ARGUMENT("motor direction must be 1 or -1");
    }
    servoMotorCalibrationObject->setMotorDirection(motorId,
        static_cast<::ctrl::int32>(parsed));
}

/**
 * @brief Runs, confirms, and persists automatic grayscale calibration.
 * @return `true` only after the operator confirms persistence of both references.
 * @warning The line phase drives forward and backward at low calibrated power.
 */
::ctrl::boolean XWalkController::calibrateGrayscaleReferences()
{
    if (grayscaleCalibrationObject == nullptr)
    {
        output("Grayscale-calibration backend unavailable");
        return false;
    }
    picarxObject->stop();
    output("--- Automatic Grayscale and Cliff Calibration ---");
    output("Clear the steering mechanism and provide a clear calibration surface.");
    const ::ctrl::string ready = input(
        "Type 'ready' to run the steering check, or 'skip': ");
    if ((ready != "ready") && (ready != "READY"))
    {
        return false;
    }
    const ::ctrl::boolean steeringCheckCompleted =
        grayscaleCalibrationObject->runSteeringCheck();
    if (steeringCheckCompleted == false)
    {
        return false;
    }

    const ::ctrl::string lineRequest = input(
        "Place the car on the line calibration surface and type 'q', or 'skip': ");
    if ((lineRequest != "q") && (lineRequest != "Q"))
    {
        return false;
    }
    const ::ctrl::boolean lineCalibrated = grayscaleCalibrationObject->calibrateLine();
    if (lineCalibrated == false)
    {
        return false;
    }
    output(::ctrl::string("Pending line reference: ") +
        XWALK_formatValues(grayscaleCalibrationObject->result().lineReference));

    const ::ctrl::string cliffRequest = input(
        "Position sensors at the cliff threshold and type 'e', or 'skip': ");
    if ((cliffRequest != "e") && (cliffRequest != "E"))
    {
        return false;
    }
    const ::ctrl::boolean cliffCalibrated = grayscaleCalibrationObject->calibrateCliff();
    if (cliffCalibrated == false)
    {
        return false;
    }
    output(::ctrl::string("Pending cliff reference: ") +
        XWALK_formatValues(grayscaleCalibrationObject->result().cliffReference));

    const ::ctrl::string accepted = input("Persist both references? (y/n): ");
    if ((accepted != "y") && (accepted != "Y"))
    {
        return false;
    }
    grayscaleCalibrationObject->save();
    return true;
}

/**
 * @brief Calibrates one servo through repeated platform prompts.
 * @param[in] configuration Title, prompt, valid angle range, and servo identifier.
 * @return `true` after confirmation or skip, or `false` after cancellation.
 */
::ctrl::boolean XWalkController::calibrateServo(
    const XWalkServoCalibrationConfig& configuration)
{
    output(configuration.title);
    while (true)
    {
        const ::ctrl::string valueText = input(configuration.prompt);
        if ((valueText == "skip") || (valueText == "SKIP"))
        {
            return true;
        }
        const ::ctrl::float64 angle = XWALK_parseNumber(valueText, "servo calibration",
            configuration.minimumAngleDegrees, configuration.maximumAngleDegrees);
        const ::ctrl::boolean offsetApplied =
            servoMotorCalibrationObject->setServoOffset(configuration.servoId, angle);
        if (offsetApplied == false)
        {
            return false;
        }
        const ::ctrl::string response = input("Is it centered? (y/n/skip): ");
        if ((response == "y") || (response == "Y") ||
            (response == "skip") || (response == "SKIP"))
        {
            return true;
        }
    }
}

} /* namespace xwalk::ctrl */
