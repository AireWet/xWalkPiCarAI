/******************************************************************************
 * @file        xAgent_Rpi5CarComputerVision.cpp
 * @brief       Implements source-compatible computer-vision key handling.
 *
 * @details
 * Maps photograph, color, face, QR, and object-report keys onto one validated
 * provider while retaining detector modes and last reported QR data.
 *
 * @project     xWalk Firmware
 * @module      xWalkComputerVision
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

#include "xAgent_Rpi5CarComputerVision.h"

#include "xHal_Rpi5CarTrace.h"
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
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Polls and records a newly decoded QR value when enabled.
     * @param[in,out] result Result that receives the observation and change flag.
     */
    void XWalkComputerVision::pollQr(XWalkComputerVisionResult& result)
    {
        if (!qrEnabledValue)
        {
            return;
        }
        if (result.event != XWalkComputerVisionEvent::ObjectsShown)
        {
            result.observation = callbacks.observe(callbackContext);
        }
        const agent::string& qrData = result.observation.qrData;
        const agent::boolean newQrCodeDetected = static_cast<agent::boolean>(!qrData.empty() && (qrData != lastQrData));
        if (newQrCodeDetected)
        {
            lastQrData = qrData;
            result.qrChanged = true;
        }
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Applies one source-compatible interactive key.
     * @param[in] keyText One-character key, case-insensitive.
     * @return Completed action, retained modes, observation, and QR change.
     * @throws std::logic_error If `start()` has not succeeded.
     */
    XWalkComputerVisionResult XWalkComputerVision::handleKey(agent::stringview keyText)
    {
        if (!startedValue)
        {
            XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "Computer vision must be started before handling keys");
        }

        XWalkComputerVisionResult result;
        result.color = colorValue;
        result.faceEnabled = faceEnabledValue;
        result.qrEnabled = qrEnabledValue;
        const agent::boolean keyTextMatched = static_cast<agent::boolean>(keyText.size() == 1U);
        if (keyTextMatched)
        {
            char key = keyText[0U];
            if ((key >= 'A') && (key <= 'Z'))
            {
                key = static_cast<char>(key - 'A' + 'a');
            }
            if (key == 'q')
            {
                result.photoPath = callbacks.capture(callbackContext);
                result.event = XWalkComputerVisionEvent::PhotoCaptured;
            }
            else if ((key >= '0') && (key <= '6'))
            {
                colorValue = static_cast<XWalkComputerVisionColor>(key - '0');
                callbacks.setColor(callbackContext, colorValue);
                result.color = colorValue;
                result.event = XWalkComputerVisionEvent::ColorChanged;
            }
            else if (key == 'f')
            {
                faceEnabledValue = !faceEnabledValue;
                callbacks.setFace(callbackContext, faceEnabledValue);
                result.faceEnabled = faceEnabledValue;
                result.event = XWalkComputerVisionEvent::FaceChanged;
            }
            else if (key == 'r')
            {
                qrEnabledValue = !qrEnabledValue;
                callbacks.setQr(callbackContext, qrEnabledValue);
                if (!qrEnabledValue)
                {
                    lastQrData.clear();
                }
                result.qrEnabled = qrEnabledValue;
                result.event = XWalkComputerVisionEvent::QrChanged;
            }
            else if (key == 's')
            {
                result.observation = callbacks.observe(callbackContext);
                result.event = XWalkComputerVisionEvent::ObjectsShown;
            }
        }

        pollQr(result);
        const agent::boolean delayCompleted = waitAfterCommand();
        if (delayCompleted == false)
        {
            result.event = XWalkComputerVisionEvent::Cancelled;
        }
        return result;
    }

    /**
     * @brief Returns the lowercase name used by the upstream example.
     * @param[in] color Supported color-detection mode.
     * @return `close`, `red`, `orange`, `yellow`, `green`, `blue`, or `purple`.
     * @throws std::invalid_argument If `color` is outside the enumeration.
     */
    agent::string XWalkComputerVision::colorName(XWalkComputerVisionColor color)
    {
        switch (color)
        {
            case XWalkComputerVisionColor::Close:
                return "close";
            case XWalkComputerVisionColor::Red:
                return "red";
            case XWalkComputerVisionColor::Orange:
                return "orange";
            case XWalkComputerVisionColor::Yellow:
                return "yellow";
            case XWalkComputerVisionColor::Green:
                return "green";
            case XWalkComputerVisionColor::Blue:
                return "blue";
            case XWalkComputerVisionColor::Purple:
                return "purple";
            default:
                XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Computer-vision color is invalid");
        }
    }

} /* namespace xwalk::agent */
