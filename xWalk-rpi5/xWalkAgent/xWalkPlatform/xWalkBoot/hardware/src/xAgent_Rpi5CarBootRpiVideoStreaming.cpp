/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVideoStreaming.cpp
 * @brief       Composes Raspberry Pi MJPEG video streaming.
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xHal_Rpi5CarCameraStream.h"
#include "xHal_Rpi5CarCameraStreamOpenCv.h"
#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarTrace.h"

namespace xwalk::agent
{

    /**
     * @brief Returns the platform monotonic time used by the MJPEG transport.
     * @param[in] context Unused nullable callback context.
     * @return Monotonic elapsed time in milliseconds.
     */
    agent::uint64 XWalkBootRpi::videoStreamClock(agent::contextpointer context) noexcept
    {
        static_cast<void>(context);
        return hal::common::monotonicMicroseconds() / 1'000U;
    }

    /**
     * @brief Composes and exposes the configured camera-to-MJPEG stream.
     * @param[in] parameters Non-null application callback and configuration context.
     * @return Status returned by the synchronous application callback.
     * @throws std::invalid_argument If a deployment value is invalid.
     */
    agent::int32 XWalkBootRpi::runVideoStreaming(const xAgentContext& parameters)
    {
        hal::XWalkConfigStore& config = *parameters.config;
        hal::XWalkCameraStreamConfiguration cameraConfiguration;
        cameraConfiguration.backend = config.get("video_stream_camera_backend", "libcamera");
        cameraConfiguration.source = config.get("video_stream_camera_device", "csi");
        cameraConfiguration.widthPixels =
            parseUnsigned(config.get("video_stream_width", "640"), "video_stream_width", 7'680U);
        cameraConfiguration.heightPixels =
            parseUnsigned(config.get("video_stream_height", "480"), "video_stream_height", 4'320U);
        cameraConfiguration.jpegQuality =
            parseUnsigned(config.get("video_stream_jpeg_quality", "80"), "video_stream_jpeg_quality", 100U);
        cameraConfiguration.readTimeoutMs =
            parseUnsigned(config.get("video_stream_read_timeout_ms", "1000"), "video_stream_read_timeout_ms", 60'000U);

        XWalkMjpegHttpConfiguration httpConfiguration;
        httpConfiguration.stream.bindAddress = config.get("video_stream_bind_address", "127.0.0.1");
        httpConfiguration.stream.port =
            parseUnsigned(config.get("video_stream_port", "8080"), "video_stream_port", 65'535U);
        httpConfiguration.stream.maximumClients =
            parseUnsigned(config.get("video_stream_maximum_clients", "4"), "video_stream_maximum_clients", 32U);
        httpConfiguration.stream.queueCapacity =
            parseUnsigned(config.get("video_stream_queue_capacity", "2"), "video_stream_queue_capacity", 16U);

        hal::XWalkCameraStreamOpenCv cameraBackend;
        hal::XWalkCameraStream camera(&cameraBackend, cameraBackend.callbacks(), cameraConfiguration);
        XWalkVideoStreaming streaming(camera, &videoStreamClock, httpConfiguration);
        XWalkBootServices services{};
        services.videoStreaming = &streaming;
        XWALK_RPIAGENT_TRACE_UID1(
            RPIAGENT .087, "Boot composing video stream on port %u", httpConfiguration.stream.port);
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
