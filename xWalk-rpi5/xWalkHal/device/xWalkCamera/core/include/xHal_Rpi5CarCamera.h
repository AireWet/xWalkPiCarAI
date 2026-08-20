/******************************************************************************
 * @file        xHal_Rpi5CarCamera.h
 * @brief       Declares the backend-neutral xWalk still camera.
 *
 * @details
 * Validates bounded capture requests and forwards them synchronously to one
 * caller-owned camera backend.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera
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

#ifndef XHAL_RPI5CAR_CAMERA_H
#define XHAL_RPI5CAR_CAMERA_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCameraTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal
{

    /** @brief Validates a bounded camera source without opening a device or URL. */
    boolean validCameraSourceString(stringview source) noexcept;

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkCamera
     * @brief Coordinates synchronous still-image capture without owning hardware.
     */
    class XWalkCamera final
    {
        private:
            /** @brief Nullable non-owning backend context. */
            contextpointer backendContext{nullptr};
            /** @brief Required synchronous capture callback. */
            cameracapturecallback captureCallback{nullptr};
            /** @brief Owned validated capture settings. */
            XWalkCameraConfiguration configurationValue{};

        protected:
            /** @brief Validates the callback and bounded capture settings. */
            static void validate(cameracapturecallback capture, const XWalkCameraConfiguration& configuration);

        public:
            /**
             * @brief Constructs a camera around one caller-owned backend.
             * @param[in,out] context Nullable context that outlives this camera.
             * @param[in] captureOperation Non-null synchronous capture callback.
             * @param[in] configuration Bounded still-image settings copied by value.
             */
            XWalkCamera(contextpointer context,
                        cameracapturecallback captureOperation,
                        const XWalkCameraConfiguration& configuration = {});

            /** @brief Releases no caller-owned backend resources. */
            ~XWalkCamera();

            XWalkCamera(const XWalkCamera&) = delete;
            XWalkCamera& operator=(const XWalkCamera&) = delete;
            XWalkCamera(XWalkCamera&&) = delete;
            XWalkCamera& operator=(XWalkCamera&&) = delete;

            /**
             * @brief Captures one JPEG image at a caller-selected destination.
             * @param[in] outputPath Non-empty single-line destination path.
             * @return Owned destination path after successful capture.
             * @throws std::runtime_error If the backend reports capture failure.
             */
            string capture(stringview outputPath);

            /**
             * @brief Attempts one JPEG capture for a caller that permits no image.
             * @param[in] outputPath Non-empty single-line destination path.
             * @return `true` after successful capture; otherwise `false` when the
             * backend cannot produce an image.
             * @throws std::invalid_argument If the output path is empty or contains
             * a line terminator.
             */
            boolean tryCapture(stringview outputPath);

            /**
             * @brief Converts a deployment connection name to its typed value.
             * @param[in] connection Exact lowercase `csi` or `usb` text.
             * @return Parsed physical connection.
             * @throws std::invalid_argument If the value is unsupported.
             */
            static XWalkCameraConnection connectionFromString(stringview connection);
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_CAMERA_H */
