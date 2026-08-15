/******************************************************************************
 * @file        xControllerComputerVisionHandler.cpp
 * @brief       Implements the ComputerVisionHandler command responsibility.
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
#include "xControllerParsing.h"
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
     * @brief Runs interactive computer vision ported from `7.computer_vision.py`.
     * @param[in] request Validated empty request.
     * @return Zero after exit or cancellation, two when camera start fails, or
     * three without an Agent.
     */
    ::ctrl::int32 XWalkController::XWALK_handlerComputerVision(const XWalkNoArgumentRequest& request)
    {
        static_cast<void>(request);
        if (computerVisionObject == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Computer-vision backend unavailable");
            return 3;
        }
        const ::ctrl::boolean started = computerVisionObject->start();
        if (started == false)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Computer-vision camera could not be started");
            return 2;
        }

        XWALK_CTRL_TRACE_UID0(CTRL .045,
                              "Computer vision keys: q photo; 1-6 colors; 0 color off; "
                              "r QR; f face; s status; x exit.");
        const ::ctrl::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const ::ctrl::boolean operationAllowed = static_cast<::ctrl::boolean>(operationMayContinue());
            if (operationAllowed == false)
            {
                break;
            }
            const ::ctrl::string key = input("vision> ");
            if ((key == "x") || (key == "X") || (key == "exit") || (key == "stop") || (key == "skip"))
            {
                break;
            }
            const agent::XWalkComputerVisionResult result = computerVisionObject->handleKey(key);
            if (result.event == agent::XWalkComputerVisionEvent::PhotoCaptured)
            {
                XWALK_CTRL_TRACE_UID1(CTRL .046, "photo save as %s", result.photoPath.c_str());
            }
            else if (result.event == agent::XWalkComputerVisionEvent::ColorChanged)
            {
                XWALK_CTRL_TRACE_UID1(
                    CTRL .047, "Color detect : %s", agent::XWalkComputerVision::colorName(result.color).c_str());
            }
            else if (result.event == agent::XWalkComputerVisionEvent::FaceChanged)
            {
                XWALK_CTRL_TRACE_UID1(CTRL .048, "Face Detect:%s", result.faceEnabled ? "True" : "False");
            }
            else if (result.event == agent::XWalkComputerVisionEvent::QrChanged)
            {
                XWALK_CTRL_TRACE_UID1(
                    CTRL .049, "%s", result.qrEnabled ? "Waitting for QR code" : "QRcode Detect: close");
            }
            else if (result.event == agent::XWalkComputerVisionEvent::ObjectsShown)
            {
                if (result.color != agent::XWalkComputerVisionColor::Close)
                {
                    XWALK_CTRL_TRACE_UID2(CTRL .050,
                                          "%s%s",
                                          result.observation.color.count == 0U ? "Color Detect: " : "[Color Detect] ",
                                          result.observation.color.count == 0U
                                              ? "None"
                                              : XWALK_FORMAT_DETECTION(result.observation.color).c_str());
                }
                if (result.faceEnabled)
                {
                    XWALK_CTRL_TRACE_UID2(CTRL .051,
                                          "%s%s",
                                          result.observation.face.count == 0U ? "Face Detect: " : "[Face Detect] ",
                                          result.observation.face.count == 0U
                                              ? "None"
                                              : XWALK_FORMAT_DETECTION(result.observation.face).c_str());
                }
            }
            if (result.qrChanged)
            {
                XWALK_CTRL_TRACE_UID1(CTRL .052, "QR code:%s", result.observation.qrData.c_str());
            }
            if (result.event == agent::XWalkComputerVisionEvent::Cancelled)
            {
                break;
            }
        }
        computerVisionObject->stop();
        XWALK_CTRL_TRACE_UID0(CTRL .053, "Computer vision stopped");
        return 0;
    }

} /* namespace xwalk::ctrl */
