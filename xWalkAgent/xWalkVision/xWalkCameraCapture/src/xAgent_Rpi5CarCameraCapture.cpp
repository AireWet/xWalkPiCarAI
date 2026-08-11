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
namespace xwalk::agent {

/** @brief Binds one camera and one reusable output path. */
XWalkCameraCapture::XWalkCameraCapture(hal::XWalkCamera &camera,
                                       agent::stringview outputPath)
    : cameraObject(&camera), outputPathValue(outputPath) {
  const agent::boolean outputPathEmpty =
      static_cast<agent::boolean>(outputPathValue.empty());
  if (outputPathEmpty) {
    XWALK_RPIAGENT_ERROR(XWALK_INVAL,
                         "Camera capture output path must not be empty");
  }
}

/** @brief Releases no caller-owned camera resource. */
XWalkCameraCapture::~XWalkCameraCapture() = default;

/** @brief Captures one image and returns its owned destination path. */
agent::string XWalkCameraCapture::capture() {
  return cameraObject->capture(outputPathValue);
}

/** @brief Adapts this object to a voice-active image callback. */
agent::string XWalkCameraCapture::captureImage(agent::contextpointer context) {
  if (context == nullptr) {
    XWALK_RPIAGENT_ERROR(XWALK_INVAL,
                         "Camera capture Agent context must not be null");
  }
  return static_cast<XWalkCameraCapture *>(context)->capture();
}

} /* namespace xwalk::agent */
