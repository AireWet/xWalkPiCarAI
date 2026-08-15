/******************************************************************************
 * @file        xHal_Rpi5CarCameraLinux.h
 * @brief       Declares Linux CSI and USB still-image capture.
 *
 * @details
 * Runs a configured capture executable without shell interpretation and
 * verifies that it produced one bounded regular JPEG file.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera Linux Backend
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

#ifndef XHAL_RPI5CAR_CAMERA_LINUX_H
#define XHAL_RPI5CAR_CAMERA_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCamera.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkCameraLinux
     * @brief Owns Linux process configuration for one CSI or USB camera.
     */
    class XWalkCameraLinux final
    {
        private:
            /** @brief Selected physical camera connection. */
            XWalkCameraConnection connectionValue{XWalkCameraConnection::Csi};
            /** @brief Owned non-empty capture executable name or path. */
            string executableName{};
            /** @brief Owned V4L2 device path used only for USB capture. */
            string usbDevicePath{};

        protected:
            /** @brief Converts callback context into its required backend. */
            static XWalkCameraLinux& backend(contextpointer context);

            /** @brief Runs the selected capture process and verifies its output. */
            static boolean
            captureImage(contextpointer context, stringview outputPath, const XWalkCameraConfiguration& configuration);

            /** @brief Runs one CSI capture process without a shell. */
            boolean captureCsi(stringview outputPath, const XWalkCameraConfiguration& configuration) const;

            /** @brief Runs one USB V4L2 capture process without a shell. */
            boolean captureUsb(stringview outputPath, const XWalkCameraConfiguration& configuration) const;

            /**
             * @brief Waits within one deadline and validates the capture-process exit status.
             * @param[in] processId Positive Linux process identifier returned by `fork()`.
             * @param[in] timeoutMs Maximum wait in milliseconds before the child is terminated.
             * @return `true` only when the child exits successfully before the deadline.
             */
            static boolean waitForProcess(int32 processId, uint32 timeoutMs) noexcept;

            /** @brief Confirms one non-empty JPEG-sized regular output file. */
            static boolean validOutput(stringview outputPath);

        public:
            /**
             * @brief Constructs one Linux camera backend.
             * @param[in] connection CSI or USB connection selected by deployment.
             * @param[in] executable Non-empty `rpicam-still` or `ffmpeg` executable.
             * @param[in] usbDevice V4L2 path required for USB capture.
             */
            XWalkCameraLinux(XWalkCameraConnection connection,
                             stringview executable,
                             stringview usbDevice = "/dev/video0");

            /** @brief Releases no process or camera resource while idle. */
            ~XWalkCameraLinux();

            XWalkCameraLinux(const XWalkCameraLinux&) = delete;
            XWalkCameraLinux& operator=(const XWalkCameraLinux&) = delete;
            XWalkCameraLinux(XWalkCameraLinux&&) = delete;
            XWalkCameraLinux& operator=(XWalkCameraLinux&&) = delete;

            /** @brief Returns the capture callback requiring this object as context. */
            cameracapturecallback callback() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_CAMERA_LINUX_H */
