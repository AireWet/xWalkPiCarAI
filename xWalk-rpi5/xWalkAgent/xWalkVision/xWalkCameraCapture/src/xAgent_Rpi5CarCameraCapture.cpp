/******************************************************************************
 * @file        xAgent_Rpi5CarCameraCapture.cpp
 * @brief       Implements voice-image camera adaptation.
 *
 * @project     xWalk Firmware
 * @module      xWalkCameraCapture
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarCameraCapture.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::agent
{

    /** @brief Binds one camera and one reusable output path. */
    XWalkCameraCapture::XWalkCameraCapture(hal::XWalkCamera& camera, agent::stringview outputPath)
        : cameraObject(&camera), outputPathValue(outputPath)
    {
        const agent::boolean outputPathEmpty = static_cast<agent::boolean>(outputPathValue.empty());
        if (outputPathEmpty)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Camera capture output path must not be empty");
        }
    }

    /** @brief Releases no caller-owned camera resource. */
    XWalkCameraCapture::~XWalkCameraCapture() = default;

    /** @brief Captures one image and returns its owned destination path. */
    agent::string XWalkCameraCapture::capture()
    {
        static_cast<void>(cameraObject->capture(outputPathValue));
        return completedCapture();
    }

    /**
     * @brief Reports successful capture and returns the configured path.
     * @return Owned configured image destination.
     */
    agent::string XWalkCameraCapture::completedCapture() const
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .020, "Voice image capture completed");
        return outputPathValue;
    }

    /**
     * @brief Adapts this object to a voice-active image callback.
     * @param[in,out] context Non-null pointer to a live capture Agent.
     * @return Captured image path, or an empty string when the optional backend
     * capture fails.
     */
    agent::string XWalkCameraCapture::captureImage(agent::contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Camera capture Agent context must not be null");
        }
        XWalkCameraCapture& captureAgent = *static_cast<XWalkCameraCapture*>(context);
        const agent::boolean imageCaptured = captureAgent.cameraObject->tryCapture(captureAgent.outputPathValue);
        if (imageCaptured == false)
        {
            XWALK_RPIAGENT_WARNING(XWALK_RUNTIME, "Voice request is continuing without a camera image");
            return {};
        }
        return captureAgent.completedCapture();
    }

} /* namespace xwalk::agent */
