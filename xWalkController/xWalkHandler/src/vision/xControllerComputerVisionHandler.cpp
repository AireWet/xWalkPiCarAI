/******************************************************************************
 * @file        xControllerComputerVisionHandler.cpp
 * @brief       Implements the ComputerVisionHandler command responsibility.
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

/**
 * @brief Runs interactive computer vision ported from `7.computer_vision.py`.
 * @param[in] request Validated empty request.
 * @return Zero after exit or cancellation, two when camera start fails, or three without an Agent.
 */
::ctrl::int32 XWalkController::XWALK_handlerComputerVision(
    const XWalkNoArgumentRequest& request)
{
    static_cast<void>(request);
    if (computerVisionObject == nullptr)
    {
        output("Computer-vision backend unavailable");
        return 3;
    }
    const ::ctrl::boolean started = computerVisionObject->start();
    if (started == false)
    {
        output("Computer-vision camera could not be started");
        return 2;
    }

    output("Computer vision keys: q photo; 1-6 colors; 0 color off; "
        "r QR; f face; s status; x exit.");
    const ::ctrl::boolean processingLoopRequested{true};
    while (processingLoopRequested)
    {
        const ::ctrl::boolean operationAllowed =
            static_cast<::ctrl::boolean>(
                operationMayContinue());
        if (operationAllowed == false)
        {
            break;
        }
        const ::ctrl::string key = input("vision> ");
        if ((key == "x") || (key == "X") || (key == "exit") ||
            (key == "stop") || (key == "skip"))
        {
            break;
        }
        const agent::XWalkComputerVisionResult result = computerVisionObject->handleKey(key);
        if (result.event == agent::XWalkComputerVisionEvent::PhotoCaptured)
        {
            output("photo save as " + result.photoPath);
        }
        else if (result.event == agent::XWalkComputerVisionEvent::ColorChanged)
        {
            output("Color detect : " + agent::XWalkComputerVision::colorName(result.color));
        }
        else if (result.event == agent::XWalkComputerVisionEvent::FaceChanged)
        {
            output(::ctrl::string("Face Detect:") + (result.faceEnabled ? "True" : "False"));
        }
        else if (result.event == agent::XWalkComputerVisionEvent::QrChanged)
        {
            output(result.qrEnabled ? "Waitting for QR code" : "QRcode Detect: close");
        }
        else if (result.event == agent::XWalkComputerVisionEvent::ObjectsShown)
        {
            if (result.color != agent::XWalkComputerVisionColor::Close)
            {
                output((result.observation.color.count == 0U) ?
                    "Color Detect: None" :
                    "[Color Detect] " + XWALK_formatDetection(result.observation.color));
            }
            if (result.faceEnabled)
            {
                output((result.observation.face.count == 0U) ?
                    "Face Detect: None" :
                    "[Face Detect] " + XWALK_formatDetection(result.observation.face));
            }
        }
        if (result.qrChanged)
        {
            output("QR code:" + result.observation.qrData);
        }
        if (result.event == agent::XWalkComputerVisionEvent::Cancelled)
        {
            break;
        }
    }
    computerVisionObject->stop();
    output("Computer vision stopped");
    return 0;
}

} /* namespace xwalk::ctrl */
