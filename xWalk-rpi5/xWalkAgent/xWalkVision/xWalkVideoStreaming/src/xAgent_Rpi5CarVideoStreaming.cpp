/******************************************************************************
 * @file        xAgent_Rpi5CarVideoStreaming.cpp
 * @brief       Implements camera-to-MJPEG streaming coordination.
 *
 * @details
 * Couples injected JPEG acquisition to the bounded pump-driven HTTP server.
 *
 * @project     xWalk Firmware
 * @module      xWalkVideoStreaming
 *
 * @author      Joxy John
 * @date        2026-08-20
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

#include "xAgent_Rpi5CarVideoStreaming.h"

#include "xHal_Rpi5CarTrace.h"

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /**
     * @brief Validates the complete camera callback table.
     * @param[in] providerCallbacks Callback table copied by the coordinator.
     * @throws std::invalid_argument If a required callback is null.
     */
    void XWalkVideoStreaming::validateCallbacks(const XWalkVideoStreamingCallbacks& providerCallbacks)
    {
        if ((providerCallbacks.start == nullptr) || (providerCallbacks.stop == nullptr) ||
            (providerCallbacks.capture == nullptr) || (providerCallbacks.now == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Video-streaming callbacks must be complete");
        }
    }

    /**
     * @brief Starts an injected HAL encoded-camera interface.
     * @param[in,out] context Non-null non-owning pointer to a live HAL camera stream.
     * @return True when the HAL camera is active; otherwise false.
     */
    agent::boolean XWalkVideoStreaming::startHalCamera(agent::contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Video-streaming HAL camera context is null");
        }
        return static_cast<hal::XWalkCameraStream*>(context)->start();
    }

    /**
     * @brief Stops an injected HAL encoded-camera interface.
     * @param[in,out] context Nullable non-owning pointer to a live HAL camera stream.
     */
    void XWalkVideoStreaming::stopHalCamera(agent::contextpointer context) noexcept
    {
        if (context != nullptr)
        {
            static_cast<hal::XWalkCameraStream*>(context)->stop();
        }
    }

    /**
     * @brief Acquires one JPEG frame through an injected HAL camera interface.
     * @param[in,out] context Non-null non-owning pointer to a live HAL camera stream.
     * @param[out] jpeg Complete encoded JPEG bytes, or an empty vector on failure.
     * @return True when the HAL camera supplies one frame; otherwise false.
     */
    agent::boolean XWalkVideoStreaming::captureHalCamera(agent::contextpointer context, agent::bytevector& jpeg)
    {
        if (context == nullptr)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Video-streaming HAL camera context is null");
        }
        return static_cast<hal::XWalkCameraStream*>(context)->capture(jpeg);
    }

    /**
     * @brief Binds caller-owned camera operations and stream settings.
     * @param[in,out] context Optional provider context that must outlive this object.
     * @param[in] providerCallbacks Complete synchronous camera callback table.
     * @param[in] streamConfiguration Bounded HTTP listener configuration.
     */
    XWalkVideoStreaming::XWalkVideoStreaming(agent::contextpointer context,
                                             const XWalkVideoStreamingCallbacks& providerCallbacks,
                                             const XWalkMjpegHttpConfiguration& streamConfiguration)
        : callbackContext(context), callbacks(providerCallbacks), configuration(streamConfiguration)
    {
        validateCallbacks(callbacks);
        const XWalkMjpegHttpStatus configurationStatus = validateMjpegHttpConfiguration(configuration);
        if (configurationStatus != XWalkMjpegHttpStatus::Ok)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Video-streaming HTTP configuration is invalid");
        }
    }

    /**
     * @brief Binds one caller-owned HAL camera and stream settings.
     * @param[in,out] cameraStream Non-null HAL camera that must outlive this object.
     * @param[in] clock Non-null monotonic millisecond clock supplied by the composition root.
     * @param[in] streamConfiguration Bounded HTTP listener configuration to copy.
     * @throws std::invalid_argument If the clock or HTTP setting is invalid.
     */
    XWalkVideoStreaming::XWalkVideoStreaming(hal::XWalkCameraStream& cameraStream,
                                             videostreamclockcallback clock,
                                             const XWalkMjpegHttpConfiguration& streamConfiguration)
        : XWalkVideoStreaming(
              &cameraStream, {&startHalCamera, &stopHalCamera, &captureHalCamera, clock}, streamConfiguration)
    {
    }

    /**
     * @brief Stops the listener and camera without throwing.
     */
    XWalkVideoStreaming::~XWalkVideoStreaming() noexcept
    {
        stop();
    }

    /**
     * @brief Opens the camera and starts the configured HTTP listener.
     * @return True when both resources are active; otherwise false after cleanup.
     */
    agent::boolean XWalkVideoStreaming::start()
    {
        if (startedValue)
        {
            return true;
        }
        const agent::boolean cameraStarted = callbacks.start(callbackContext);
        if (cameraStarted == false)
        {
            callbacks.stop(callbackContext);
            XWALK_RPIAGENT_ERROR(XWALK_EXCEPTION, "Video-streaming camera startup failed and was stopped");
            return false;
        }
        const XWalkMjpegHttpStatus serverStatus = startMjpegHttpServer(server, stream, configuration);
        if (serverStatus != XWalkMjpegHttpStatus::Ok)
        {
            callbacks.stop(callbackContext);
            XWALK_RPIAGENT_ERROR(XWALK_EXCEPTION,
                                 "Video-streaming HTTP listener startup failed with status %d; camera was stopped",
                                 static_cast<int>(serverStatus));
            return false;
        }
        startedValue = true;
        return true;
    }

    /**
     * @brief Captures, publishes, and pumps one bounded stream iteration.
     * @return True when the camera and HTTP server remain usable.
     */
    agent::boolean XWalkVideoStreaming::step()
    {
        if (startedValue == false)
        {
            return false;
        }
        agent::bytevector jpeg;
        const agent::boolean frameCaptured = callbacks.capture(callbackContext, jpeg);
        if (frameCaptured == false)
        {
            static_cast<void>(reportMjpegCameraLoss(stream));
            stop();
            XWALK_RPIAGENT_ERROR(XWALK_EXCEPTION, "Video-streaming frame capture failed; resources were stopped");
            return false;
        }
        const XWalkMjpegStreamStatus publishStatus = publishMjpegFrame(stream, jpeg);
        const agent::uint64 nowMilliseconds = callbacks.now(callbackContext);
        const XWalkMjpegHttpStatus pumpStatus = pumpMjpegHttpServer(server, nowMilliseconds);
        const agent::boolean healthy = static_cast<agent::boolean>((publishStatus == XWalkMjpegStreamStatus::Ok) &&
                                                                   (pumpStatus == XWalkMjpegHttpStatus::Ok));
        if (healthy == false)
        {
            stop();
            XWALK_RPIAGENT_ERROR(XWALK_EXCEPTION,
                                 "Video-streaming HTTP iteration failed with publish status %d and pump status %d; "
                                 "resources were stopped",
                                 static_cast<int>(publishStatus),
                                 static_cast<int>(pumpStatus));
        }
        return healthy;
    }

    /**
     * @brief Idempotently closes clients, listener, queues, and camera.
     */
    void XWalkVideoStreaming::stop() noexcept
    {
        if (startedValue)
        {
            static_cast<void>(stopMjpegHttpServer(server));
            callbacks.stop(callbackContext);
            startedValue = false;
        }
    }

    /**
     * @brief Returns the active listener port, or zero while stopped.
     * @return Listener port in host byte order, or zero.
     */
    agent::uint32 XWalkVideoStreaming::port() const noexcept
    {
        return mjpegHttpServerPort(server);
    }

    /**
     * @brief Reports whether camera and listener startup completed.
     * @return True only while both resources are retained.
     */
    agent::boolean XWalkVideoStreaming::started() const noexcept
    {
        return startedValue;
    }

} /* namespace xwalk::agent */
