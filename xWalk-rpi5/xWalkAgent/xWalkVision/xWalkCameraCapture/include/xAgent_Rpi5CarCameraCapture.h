/******************************************************************************
 * @file        xAgent_Rpi5CarCameraCapture.h
 * @brief       Declares the xWalk voice-image capture Agent.
 *
 * @details
 * Adapts a caller-owned HAL camera and configured output path to the callback
 * contract consumed by the voice-active-car coordinator.
 *
 * @project     xWalk Firmware
 * @module      xWalkCameraCapture
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_CAMERA_CAPTURE_H
#define XAGENT_RPI5CAR_CAMERA_CAPTURE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCamera.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkCameraCapture
     * @brief Supplies bounded still images to application coordinators.
     */
    class XWalkCameraCapture final
    {
        private:
            /** @brief Non-owning camera pointer that remains valid for this object. */
            hal::XWalkCamera* cameraObject{nullptr};
            /** @brief Owned non-empty JPEG destination path. */
            agent::string outputPathValue{};

        public:
            /**
             * @brief Binds one camera and one reusable output path.
             * @param[in] camera Caller-owned HAL camera that outlives this object.
             * @param[in] outputPath Non-empty destination overwritten by each capture.
             */
            XWalkCameraCapture(hal::XWalkCamera& camera, agent::stringview outputPath);

            /** @brief Releases no caller-owned camera resource. */
            ~XWalkCameraCapture();

            XWalkCameraCapture(const XWalkCameraCapture&) = delete;
            XWalkCameraCapture& operator=(const XWalkCameraCapture&) = delete;
            XWalkCameraCapture(XWalkCameraCapture&&) = delete;
            XWalkCameraCapture& operator=(XWalkCameraCapture&&) = delete;

            /** @brief Captures one image and returns its owned destination path. */
            agent::string capture();

            /**
             * @brief Adapts this object to a voice-active image callback.
             * @param[in,out] context Non-null pointer to a live capture Agent.
             * @return Captured image path.
             */
            static agent::string captureImage(agent::contextpointer context);
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_CAMERA_CAPTURE_H */
