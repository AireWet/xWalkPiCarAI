/******************************************************************************
 * @file        xAgent_Rpi5CarControllerCommands.cpp
 * @brief       Implements PiCar-X CLI command execution.
 *
 * @details
 * Coordinates movement, foreground line tracking, preset actions, sensors, audio, and calibration operations.
 *
 * @project     xWalk Firmware
 * @module      xWalkController
 *
 * @author      Joxy John
 * @date        2026-07-31
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

#include "xAgent_Rpi5CarController.h"
#include "xAgent_Rpi5CarControllerHelp.h"
#include "xAgent_Rpi5CarPicarxSafetyGuard.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Executes one CLI command.
 * @param[in] arguments Command arguments excluding the executable name.
 * @return Zero on success or three when a command-specific backend is unavailable.
 */
hal::int32 XWalkController::run(const hal::stringvector& arguments)
{
    if (arguments.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI command is required");
    }
    if ((arguments[0U] == "-h") || (arguments[0U] == "--help") ||
        (arguments[0U] == "help"))
    {
        output(usage());
        return 0;
    }
    if (arguments[0U] == "spi")
    {
        return executeSpi(arguments);
    }
    if (arguments[0U] == "doctor")
    {
        return executeDoctor(arguments);
    }
    if (picarxObject == nullptr)
    {
        output("PiCar-X backend unavailable");
        return 3;
    }
    picarxObject->clearEmergencyStop();
    XWalkPicarxSafetyGuard safetyGuard(*picarxObject);
    return executePicarxCommand(arguments);
}

/**
 * @brief Dispatches one command while a command-scope safety guard is active.
 * @param[in] arguments Validated non-empty command arguments.
 * @return Zero on success or three when a command-specific backend is unavailable.
 */
hal::int32 XWalkController::executePicarxCommand(const hal::stringvector& arguments)
{
    if (arguments[0U] == "move")
    {
        return executeMove(arguments);
    }
    if (arguments[0U] == "turn")
    {
        return executeTurn(arguments);
    }
    if (arguments[0U] == "cam")
    {
        return executeCamera(arguments);
    }
    if (arguments[0U] == "sensor")
    {
        return executeSensor(arguments);
    }
    if (arguments[0U] == "line-track")
    {
        return executeLineTracking(arguments);
    }
    if (arguments[0U] == "self-drive")
    {
        return executeSelfDrive(arguments);
    }
    if (arguments[0U] == "sound")
    {
        return executeSound(arguments);
    }
    if (arguments[0U] == "voice-chat")
    {
        return executeVoiceChat(arguments);
    }
    if ((arguments[0U] == "voice-active-car") ||
        (arguments[0U] == "voice-active-car-gpt"))
    {
        return executeVoiceActiveCar(arguments);
    }
    if (arguments[0U] == "voice-controlled-car")
    {
        return executeVoiceControlledCar(arguments);
    }
    if (arguments[0U] == "voice-prompt-car")
    {
        return executeVoicePromptCar(arguments);
    }
    if (arguments[0U] == "calibrate")
    {
        return executeCalibration(arguments);
    }
    XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI command is not supported");
}

/**
 * @brief Executes one sensor-aware voice-active-car command.
 * @param[in] arguments Command name followed by exactly `start` or `stop`.
 * @return Zero on completion or three when the selected backend is unavailable.
 */
hal::int32 XWalkController::executeVoiceActiveCar(
    const hal::stringvector& arguments)
{
    if (arguments.size() != 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "voice-active-car requires exactly start or stop");
    }
    if ((arguments[1U] != "start") && (arguments[1U] != "stop"))
    {
        XHAL_THROW_INVALID_ARGUMENT("voice-active-car action must be start or stop");
    }
    if (voiceActiveCarObject == nullptr)
    {
        output("Voice-active-car backend unavailable");
        return 3;
    }
    if (arguments[1U] == "stop")
    {
        voiceActiveCarObject->stop();
        output("Voice-active car stopped");
        return 0;
    }
    return voiceActiveCarObject->run();
}

/** @brief Executes one wake-word voice-controlled-car command. */
hal::int32 XWalkController::executeVoiceControlledCar(
    const hal::stringvector& arguments)
{
    if (arguments.size() != 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "voice-controlled-car requires exactly start or stop");
    }
    if ((arguments[1U] != "start") && (arguments[1U] != "stop"))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "voice-controlled-car action must be start or stop");
    }
    if (voiceControlledCarObject == nullptr)
    {
        output("Voice-controlled-car backend unavailable");
        return 3;
    }
    if (arguments[1U] == "stop")
    {
        voiceControlledCarObject->stop();
        output("Voice-controlled car stopped");
        return 0;
    }
    return voiceControlledCarObject->run();
}

/** @brief Executes one spoken movement-demonstration command. */
hal::int32 XWalkController::executeVoicePromptCar(
    const hal::stringvector& arguments)
{
    if (arguments.size() != 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT("voice-prompt-car requires exactly start or stop");
    }
    if ((arguments[1U] != "start") && (arguments[1U] != "stop"))
    {
        XHAL_THROW_INVALID_ARGUMENT("voice-prompt-car action must be start or stop");
    }
    if (voicePromptCarObject == nullptr)
    {
        output("Voice-prompt-car backend unavailable");
        return 3;
    }
    if (arguments[1U] == "stop")
    {
        voicePromptCarObject->stop();
        output("Voice-prompt car stopped");
        return 0;
    }
    return voicePromptCarObject->run();
}

/**
 * @brief Executes the foreground local voice-chatbot command.
 * @param[in] arguments Exactly one `start` or `stop` action.
 * @return Zero after cancellation or three when the chatbot is unavailable.
 */
hal::int32 XWalkController::executeVoiceChat(const hal::stringvector& arguments)
{
    if (arguments.size() != 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT("voice-chat requires exactly start or stop");
    }
    if ((arguments[1U] != "start") && (arguments[1U] != "stop"))
    {
        XHAL_THROW_INVALID_ARGUMENT("voice-chat action must be start or stop");
    }
    if (localVoiceChatbotObject == nullptr)
    {
        output("Local voice-chatbot backend unavailable");
        return 3;
    }
    if (arguments[1U] == "stop")
    {
        localVoiceChatbotObject->stop();
        output("Local voice chatbot stopped");
        return 0;
    }
    return localVoiceChatbotObject->run();
}

/**
 * @brief Returns Linux-style command help with examples.
 * @return Owned multi-line help text describing commands, options, and examples.
 */
hal::string XWalkController::usage()
{
    return XAGENT_RPI5CAR_CONTROLLER_HELP;
}

/**
 * @brief Executes one bounded full-duplex SPI transfer.
 * @param[in] arguments Exact `spi transfer` action and one hexadecimal payload.
 * @return Zero after printing received bytes, or three when SPI is unavailable.
 */
hal::int32 XWalkController::executeSpi(const hal::stringvector& arguments)
{
    if ((arguments.size() != 3U) || (arguments[1U] != "transfer"))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "spi requires transfer followed by one hexadecimal payload");
    }
    if (spiTransferObject == nullptr)
    {
        output("SPI backend unavailable");
        return 3;
    }
    const hal::bytevector transmitData = parseHexBytes(arguments[2U]);
    output(formatHexBytes(spiTransferObject->transfer(transmitData)));
    return 0;
}

/**
 * @brief Prints one passive hardware preflight report.
 * @param[in] arguments Doctor command without additional arguments.
 * @return Zero when every reported check passes; otherwise two.
 */
hal::int32 XWalkController::executeDoctor(const hal::stringvector& arguments)
{
    if (arguments.size() != 1U)
    {
        XHAL_THROW_INVALID_ARGUMENT("doctor accepts no additional arguments");
    }
    if (doctorLinesObject == nullptr)
    {
        output("Doctor backend unavailable");
        return 3;
    }
    hal::boolean passed = true;
    for (const hal::string& line : *doctorLinesObject)
    {
        output(line);
        if (line.find("[FAIL]") != hal::string::npos)
        {
            passed = false;
        }
    }
    return passed ? 0 : 2;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Reports whether the active command may continue and latches emergency stop otherwise.
 * @return `true` while the application permits another bounded step; otherwise `false`.
 */
hal::boolean XWalkController::operationMayContinue()
{
    if (callbacks.continueOperation(callbackContext))
    {
        return true;
    }
    static_cast<void>(picarxObject->emergencyStop());
    return false;
}

/**
 * @brief Performs a cancellable delay using bounded application-owned slices.
 * @param[in] durationMs Total requested delay in milliseconds.
 * @return `true` after the complete delay; `false` after cancellation and emergency stop.
 */
hal::boolean XWalkController::delayWhileOperationRequested(hal::uint32 durationMs)
{
    constexpr hal::uint32 cancellationIntervalMs{20U};
    hal::uint32 remainingMs = durationMs;
    while (remainingMs > 0U)
    {
        if (!operationMayContinue())
        {
            return false;
        }
        const hal::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ?
            remainingMs : cancellationIntervalMs;
        delay(sliceMs);
        remainingMs -= sliceMs;
    }
    return operationMayContinue();
}

/**
 * @brief Executes the move command.
 * @param[in] arguments Move action and named options.
 * @return Zero after movement and the final stop complete.
 */
hal::int32 XWalkController::executeMove(const hal::stringvector& arguments)
{
    if (arguments.size() < 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT("move requires forward or backward");
    }
    const controlleroptions options = parseOptions(arguments, 2U);
    validateOptions(options, {"speed", "duration"});
    const hal::float64 speed = parseNumber(optionValue(options, "speed", "50", false),
        "move speed", 0.0, 100.0);
    const hal::float64 duration = parseNumber(optionValue(options, "duration", "1.0", false),
        "move duration", 0.0, 4'294'967.295);
    if (!operationMayContinue())
    {
        return 0;
    }
    if (arguments[1U] == "forward")
    {
        picarxObject->forward(speed);
    }
    else if (arguments[1U] == "backward")
    {
        picarxObject->backward(speed);
    }
    else
    {
        XHAL_THROW_INVALID_ARGUMENT("move action must be forward or backward");
    }
    static_cast<void>(delayWhileOperationRequested(durationMilliseconds(duration)));
    picarxObject->stop();
    return 0;
}

/**
 * @brief Executes the turn command.
 * @param[in] arguments Turn direction and optional steering angle.
 * @return Zero after the fixed turn sequence and final stop complete.
 */
hal::int32 XWalkController::executeTurn(const hal::stringvector& arguments)
{
    if (arguments.size() < 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT("turn requires left or right");
    }
    const controlleroptions options = parseOptions(arguments, 2U);
    validateOptions(options, {"angle"});
    const hal::float64 angle = parseNumber(optionValue(options, "angle", "30", false),
        "turn angle", 0.0, 30.0);
    hal::float64 signedAngle{};
    if (arguments[1U] == "right")
    {
        signedAngle = angle;
    }
    else if (arguments[1U] == "left")
    {
        signedAngle = -angle;
    }
    else
    {
        XHAL_THROW_INVALID_ARGUMENT("turn direction must be left or right");
    }
    if (!operationMayContinue())
    {
        return 0;
    }
    picarxObject->setDirectionServoAngle(signedAngle);
    if (!delayWhileOperationRequested(300U))
    {
        return 0;
    }
    picarxObject->forward(30.0);
    if (!delayWhileOperationRequested(800U))
    {
        return 0;
    }
    picarxObject->setDirectionServoAngle(0.0);
    static_cast<void>(delayWhileOperationRequested(300U));
    picarxObject->stop();
    return 0;
}

/**
 * @brief Executes the camera command.
 * @param[in] arguments Camera axis and required angle option.
 * @return Zero after the servo command completes.
 */
hal::int32 XWalkController::executeCamera(const hal::stringvector& arguments)
{
    if (arguments.size() < 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT("cam requires pan or tilt");
    }
    const controlleroptions options = parseOptions(arguments, 2U);
    validateOptions(options, {"angle"});
    const hal::string angleText = optionValue(options, "angle", {}, true);
    if (arguments[1U] == "pan")
    {
        picarxObject->setCameraPanAngle(parseNumber(angleText, "camera pan angle", -90.0, 90.0));
    }
    else if (arguments[1U] == "tilt")
    {
        picarxObject->setCameraTiltAngle(parseNumber(angleText, "camera tilt angle", -35.0, 65.0));
    }
    else
    {
        XHAL_THROW_INVALID_ARGUMENT("camera action must be pan or tilt");
    }
    return 0;
}

/**
 * @brief Executes the sensor command.
 * @param[in] arguments Sensor type without named options.
 * @return Zero after sensor output completes.
 */
hal::int32 XWalkController::executeSensor(const hal::stringvector& arguments)
{
    if (arguments.size() != 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT("sensor requires exactly one type");
    }
    if (arguments[1U] == "distance")
    {
        const hal::float64 distanceCm = picarxObject->distance();
        output((distanceCm == 0.0) ? hal::string("None") : formatOneDecimal(distanceCm));
        return 0;
    }
    if (arguments[1U] != "grayscale")
    {
        XHAL_THROW_INVALID_ARGUMENT("sensor type must be distance or grayscale");
    }

    delay(300U);
    hal::fixedarray<hal::linetrackervalues, 5U> samples{};
    hal::size sampleCount{};
    const hal::linetrackervalues poisoned{2'571, 3'085, 3'599};
    for (hal::uint32 attempt = 0U; attempt < 5U; ++attempt)
    {
        const hal::linetrackervalues data = picarxObject->grayscaleData();
        if ((data != poisoned) && (data[0U] < 2'000))
        {
            samples[sampleCount] = data;
            ++sampleCount;
        }
        delay(100U);
    }

    hal::linetrackervalues data{};
    if (sampleCount > 0U)
    {
        hal::linetrackervalues reference{};
        for (hal::uint32 channel = 0U; channel < 3U; ++channel)
        {
            hal::int32 sum{};
            for (hal::size sample = 0U; sample < sampleCount; ++sample)
            {
                sum += samples[sample][channel];
            }
            reference[channel] = sum / static_cast<hal::int32>(sampleCount);
        }
        picarxObject->setGrayscaleReference(reference);
        output(hal::string("Auto ref: ") + formatValues(reference));
        data = samples[sampleCount - 1U];
    }
    else
    {
        output("WARNING: ADC may be corrupted");
    }
    output(hal::string("Grayscale: ") + formatValues(data));
    output(hal::string("Line status: ") + formatStatus(picarxObject->lineStatus(data)));
    output(hal::string("Cliff detected: ") + (picarxObject->cliffStatus(data) ? "true" : "false"));
    return 0;
}

/**
 * @brief Executes foreground line-tracking start or immediate stop.
 * @param[in] arguments Exactly one start or stop action.
 * @return Zero after tracking or stopping completes; three when the coordinator is unavailable.
 */
hal::int32 XWalkController::executeLineTracking(const hal::stringvector& arguments)
{
    if (arguments.size() != 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT("line-track requires exactly start or stop");
    }
    if ((arguments[1U] != "start") && (arguments[1U] != "stop"))
    {
        XHAL_THROW_INVALID_ARGUMENT("line-track action must be start or stop");
    }
    if (lineTrackingObject == nullptr)
    {
        output("Line-tracking backend unavailable");
        return 3;
    }
    if (arguments[1U] == "stop")
    {
        lineTrackingObject->stop();
        output("Line tracking stopped");
        return 0;
    }

    output("Line tracking started; press Ctrl+C to stop");
    while (operationMayContinue())
    {
        static_cast<void>(lineTrackingObject->step());
    }
    lineTrackingObject->stop();
    output("Line tracking stopped");
    return 0;
}

/**
 * @brief Executes one named self-drive preset action.
 * @param[in] arguments One canonical hyphenated action or separate action words.
 * @return Zero after a supported action completes; three when the coordinator is unavailable.
 */
hal::int32 XWalkController::executeSelfDrive(const hal::stringvector& arguments)
{
    if (arguments.size() < 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT("self-drive requires an action");
    }
    hal::string action = arguments[1U];
    for (hal::size index = 2U; index < arguments.size(); ++index)
    {
        action += " ";
        action += arguments[index];
    }
    std::replace(action.begin(), action.end(), '-', ' ');
    if (selfDriveObject == nullptr)
    {
        output("Self-drive backend unavailable");
        return 3;
    }
    if (!operationMayContinue())
    {
        return 0;
    }
    if (!selfDriveObject->doAction(action))
    {
        XHAL_THROW_INVALID_ARGUMENT("self-drive action is not supported");
    }
    return 0;
}

/**
 * @brief Executes the sound command.
 * @param[in] arguments Sound action, optional file or volume, and named volume option.
 * @return Zero when accepted or three when the platform audio backend is unavailable.
 */
hal::int32 XWalkController::executeSound(const hal::stringvector& arguments)
{
    if (arguments.size() < 2U)
    {
        XHAL_THROW_INVALID_ARGUMENT("sound requires an operation");
    }
    XWalkSoundOperation operation{XWalkSoundOperation::Stop};
    hal::string filePath;
    hal::optionalfloat64 volume;
    hal::size optionIndex{2U};
    if ((arguments[1U] == "play") || (arguments[1U] == "music"))
    {
        if ((arguments.size() < 3U) || (arguments[2U].rfind("--", 0U) == 0U))
        {
            XHAL_THROW_INVALID_ARGUMENT("sound play and music require a file");
        }
        operation = (arguments[1U] == "play") ? XWalkSoundOperation::Play :
            XWalkSoundOperation::Music;
        filePath = arguments[2U];
        optionIndex = 3U;
    }
    else if (arguments[1U] == "volume")
    {
        if (arguments.size() != 3U)
        {
            XHAL_THROW_INVALID_ARGUMENT("sound volume requires one value");
        }
        operation = XWalkSoundOperation::Volume;
        volume = parseNumber(arguments[2U], "sound volume", 0.0, 100.0);
        optionIndex = arguments.size();
    }
    else if (arguments[1U] != "stop")
    {
        XHAL_THROW_INVALID_ARGUMENT("sound operation is not supported");
    }

    const controlleroptions options = parseOptions(arguments, optionIndex);
    validateOptions(options, {"volume"});
    if (options.count("volume") != 0U)
    {
        volume = parseNumber(optionValue(options, "volume", {}, true), "sound volume", 0.0, 100.0);
    }
    else if ((operation == XWalkSoundOperation::Music) && !volume.has_value())
    {
        volume = 20.0;
    }
    if (!callbacks.sound(callbackContext, operation, filePath, volume))
    {
        output("Sound backend unavailable");
        return 3;
    }
    return 0;
}

/**
 * @brief Executes interactive servo calibration.
 * @param[in] arguments Calibrate command without additional arguments.
 * @return Zero after calibration and reset complete.
 */
hal::int32 XWalkController::executeCalibration(const hal::stringvector& arguments)
{
    if (arguments.size() != 1U)
    {
        XHAL_THROW_INVALID_ARGUMENT("calibrate accepts no additional arguments");
    }
    output("=== PiCar-X Servo Calibration ===");
    output("This will help you calibrate the steering servo and camera gimbal.");
    calibrateServo("--- Steering Servo ---", "Enter steering offset (-30 to 30, or 'skip'): ",
        -30.0, 30.0, 0U);
    calibrateServo("--- Camera Pan Servo ---", "Enter camera pan offset (-90 to 90, or 'skip'): ",
        -90.0, 90.0, 1U);
    calibrateServo("--- Camera Tilt Servo ---", "Enter camera tilt offset (-35 to 65, or 'skip'): ",
        -35.0, 65.0, 2U);
    picarxObject->reset();
    const hal::string motorCorrection = input(
        "Enter motor balance correction (-100 to 100, positive reduces left, or 'skip'): ");
    if ((motorCorrection != "skip") && (motorCorrection != "SKIP"))
    {
        picarxObject->calibrateMotorSpeed(parseNumber(
            motorCorrection, "motor balance correction", -100.0, 100.0));
    }
    const hal::boolean actuatorsVerified = executeFirstRunVerification();
    picarxObject->recordCalibrationVerified(actuatorsVerified);
    calibrateGrayscaleReferences();
    output("Calibration complete!");
    return actuatorsVerified ? 0 : 2;
}

/**
 * @brief Performs capped raised-wheel motor and steering verification.
 * @return `true` only when the operator confirms every required check.
 */
hal::boolean XWalkController::executeFirstRunVerification()
{
    picarxObject->recordCalibrationVerified(false);
    output("--- Raised-Wheel Actuator Verification ---");
    output("Raise all wheels, clear the area, and be ready to stop the vehicle.");
    const hal::string readiness = input("Type 'raised' to begin low-output checks, or 'skip': ");
    if ((readiness != "raised") && (readiness != "RAISED"))
    {
        return false;
    }

    picarxObject->setMotorSpeed(1U, 100.0);
    if (!delayWhileOperationRequested(500U))
    {
        return false;
    }
    picarxObject->stop();
    const hal::string leftPassed = input("Did the left motor rotate forward? (y/n): ");
    if ((leftPassed != "y") && (leftPassed != "Y"))
    {
        return false;
    }

    picarxObject->setMotorSpeed(2U, 100.0);
    if (!delayWhileOperationRequested(500U))
    {
        return false;
    }
    picarxObject->stop();
    const hal::string rightPassed = input("Did the right motor rotate forward? (y/n): ");
    if ((rightPassed != "y") && (rightPassed != "Y"))
    {
        return false;
    }

    picarxObject->setPower(100.0);
    if (!delayWhileOperationRequested(500U))
    {
        return false;
    }
    picarxObject->stop();
    const hal::string balancePassed = input(
        "Did both motors run in the expected direction and balance? (y/n): ");
    if ((balancePassed != "y") && (balancePassed != "Y"))
    {
        return false;
    }
    const hal::string steeringPassed = input("Is the steering centered? (y/n): ");
    return (steeringPassed == "y") || (steeringPassed == "Y");
}

/**
 * @brief Samples, confirms, and persists grayscale line and cliff references.
 *
 * @details
 * Stops both motors before every prompt and performs only ADC reads while the
 * operator positions the sensor over the requested surface.
 */
void XWalkController::calibrateGrayscaleReferences()
{
    picarxObject->stop();
    output("--- Stationary Grayscale and Cliff Calibration ---");
    const hal::string lineRequest = input(
        "Place all sensors over the line and type 'sample', or 'skip': ");
    if ((lineRequest == "sample") || (lineRequest == "SAMPLE"))
    {
        const hal::linetrackervalues sample = picarxObject->grayscaleData();
        output(hal::string("Line sample: ") + formatValues(sample));
        const hal::string accepted = input("Persist this line reference? (y/n): ");
        if ((accepted == "y") || (accepted == "Y"))
        {
            picarxObject->setGrayscaleReference(sample);
        }
    }

    picarxObject->stop();
    const hal::string cliffRequest = input(
        "Position sensors at the chosen cliff threshold and type 'sample', or 'skip': ");
    if ((cliffRequest == "sample") || (cliffRequest == "SAMPLE"))
    {
        const hal::linetrackervalues sample = picarxObject->grayscaleData();
        output(hal::string("Cliff sample: ") + formatValues(sample));
        const hal::string accepted = input("Persist this cliff reference? (y/n): ");
        if ((accepted == "y") || (accepted == "Y"))
        {
            picarxObject->setCliffReference(sample);
        }
    }
}

/**
 * @brief Calibrates one servo through repeated platform prompts.
 * @param[in] title Section title written before prompting.
 * @param[in] prompt Input prompt including the supported range.
 * @param[in] minimum Inclusive calibration minimum in degrees.
 * @param[in] maximum Inclusive calibration maximum in degrees.
 * @param[in] servoId Zero for steering, one for pan, or two for tilt.
 */
void XWalkController::calibrateServo(hal::stringview title, hal::stringview prompt,
    hal::float64 minimum, hal::float64 maximum, hal::uint8 servoId)
{
    output(title);
    while (true)
    {
        const hal::string valueText = input(prompt);
        if ((valueText == "skip") || (valueText == "SKIP"))
        {
            return;
        }
        const hal::float64 angle = parseNumber(valueText, "servo calibration", minimum, maximum);
        if (servoId == 0U)
        {
            picarxObject->calibrateDirectionServo(angle);
        }
        else if (servoId == 1U)
        {
            picarxObject->calibrateCameraPanServo(angle);
        }
        else
        {
            picarxObject->calibrateCameraTiltServo(angle);
        }
        const hal::string response = input("Is it centered? (y/n/skip): ");
        if ((response == "y") || (response == "Y") ||
            (response == "skip") || (response == "SKIP"))
        {
            return;
        }
    }
}

} /* namespace xwalk::agent */
