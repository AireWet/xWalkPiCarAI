/******************************************************************************
 * @file        xHal_Rpi5CarCameraHostStub.h
 * @brief       Declares the device-free xWalkCamera host stub.
 * @project     xWalk Firmware
 * @module      xWalkCamera Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_CAMERA_HOST_STUB_H
#define XHAL_RPI5CAR_CAMERA_HOST_STUB_H
#include "xHal_Rpi5CarCamera.h"
namespace xwalk::hal::sim
{
/** @brief Records one successful camera capture entirely in memory. */
class XWalkCameraHostStub final
{
    private:
        uint32 captureCountValue{};
        string outputPathValue{};
        XWalkCameraConfiguration configurationValue{};
    public:
        static boolean capture(contextpointer context, stringview outputPath,
            const XWalkCameraConfiguration& configuration);
        uint32 captureCount() const noexcept;
        stringview outputPath() const noexcept;
        const XWalkCameraConfiguration& configuration() const noexcept;
};
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_CAMERA_HOST_STUB_H */
