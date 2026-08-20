/******************************************************************************
 * @file        xAgent_Rpi5CarVideoStreaming.h
 * @brief       Declares camera-to-MJPEG streaming coordination.
 *
 * @details
 * Coordinates one caller-owned JPEG camera provider with the bounded MJPEG
 * HTTP transport while retaining camera and scheduling ownership outside the
 * hardware-independent Agent.
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

#ifndef XAGENT_RPI5CAR_VIDEO_STREAMING_H
#define XAGENT_RPI5CAR_VIDEO_STREAMING_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarMjpegHttpServer.h"
#include "xHal_Rpi5CarCameraStream.h"

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Opens the caller-owned camera provider.
     * @param[in,out] context Nullable non-owning provider context that remains caller-owned.
     * @return True when the provider is ready for capture; otherwise false.
     */
    using videostreamstartcallback = agent::boolean (*)(agent::contextpointer context);
    /**
     * @brief Closes the caller-owned camera provider without throwing.
     * @param[in,out] context Nullable non-owning provider context that remains caller-owned.
     */
    using videostreamstopcallback = void (*)(agent::contextpointer context) noexcept;
    /**
     * @brief Acquires one complete encoded JPEG frame.
     * @param[in,out] context Nullable non-owning provider context that remains caller-owned.
     * @param[out] jpeg Complete encoded frame bytes, or an empty vector on failure.
     * @return True when one frame is available; otherwise false.
     */
    using videostreamcapturecallback = agent::boolean (*)(agent::contextpointer context, agent::bytevector& jpeg);
    /**
     * @brief Returns caller-owned monotonic time in milliseconds.
     * @param[in] context Nullable non-owning provider context that remains caller-owned.
     * @return Monotonic elapsed time in milliseconds.
     */
    using videostreamclockcallback = agent::uint64 (*)(agent::contextpointer context) noexcept;

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkVideoStreamingCallbacks
     * @brief Groups the complete camera and monotonic-clock provider boundary.
     */
    struct XWalkVideoStreamingCallbacks
    {
            /** @brief Non-null camera-start operation. */
            videostreamstartcallback start{nullptr};
            /** @brief Non-null camera-stop operation. */
            videostreamstopcallback stop{nullptr};
            /** @brief Non-null JPEG acquisition operation. */
            videostreamcapturecallback capture{nullptr};
            /** @brief Non-null monotonic clock operation. */
            videostreamclockcallback now{nullptr};
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkVideoStreaming
     * @brief Coordinates one camera with a bounded multi-client MJPEG server.
     *
     * @details
     * The object owns its socket and queue state but observes a caller-owned
     * camera callback context that must outlive it. Call `step()` repeatedly
     * from a non-safety foreground event loop.
     */
    class XWalkVideoStreaming final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Nullable non-owning camera provider context. */
            agent::contextpointer callbackContext{nullptr};
            /** @brief Complete validated camera callback table. */
            XWalkVideoStreamingCallbacks callbacks{};
            /** @brief Validated HTTP configuration copied at construction. */
            XWalkMjpegHttpConfiguration configuration{};
            /** @brief Owned synchronized MJPEG queue state. */
            XWalkMjpegStreamState stream{};
            /** @brief Owned non-blocking HTTP listener and clients. */
            XWalkMjpegHttpServer server{};
            /** @brief True while camera and listener ownership are active. */
            agent::boolean startedValue{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Validates the complete callback table. */
            static void validateCallbacks(const XWalkVideoStreamingCallbacks& providerCallbacks);

            /**
             * @brief Starts an injected HAL encoded-camera interface.
             * @param[in,out] context Non-null non-owning pointer to a live HAL camera stream.
             * @return True when the HAL camera is active; otherwise false.
             */
            static agent::boolean startHalCamera(agent::contextpointer context);

            /**
             * @brief Stops an injected HAL encoded-camera interface.
             * @param[in,out] context Nullable non-owning pointer to a live HAL camera stream.
             */
            static void stopHalCamera(agent::contextpointer context) noexcept;

            /**
             * @brief Acquires one JPEG frame through an injected HAL camera interface.
             * @param[in,out] context Non-null non-owning pointer to a live HAL camera stream.
             * @param[out] jpeg Complete encoded JPEG bytes, or an empty vector on failure.
             * @return True when the HAL camera supplies one frame; otherwise false.
             */
            static agent::boolean captureHalCamera(agent::contextpointer context, agent::bytevector& jpeg);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Binds caller-owned camera operations and stream settings.
             * @param[in,out] context Optional provider context that must outlive this object.
             * @param[in] providerCallbacks Complete synchronous camera callback table to copy.
             * @param[in] streamConfiguration Bounded HTTP listener configuration to copy.
             * @throws std::invalid_argument If a callback or HTTP setting is invalid.
             */
            XWalkVideoStreaming(agent::contextpointer context,
                                const XWalkVideoStreamingCallbacks& providerCallbacks,
                                const XWalkMjpegHttpConfiguration& streamConfiguration);

            /**
             * @brief Binds one caller-owned HAL camera and stream settings.
             * @param[in,out] cameraStream Non-null HAL camera that must outlive this object.
             * @param[in] clock Non-null monotonic millisecond clock supplied by the composition root.
             * @param[in] streamConfiguration Bounded HTTP listener configuration to copy.
             * @throws std::invalid_argument If the clock or HTTP setting is invalid.
             */
            XWalkVideoStreaming(hal::XWalkCameraStream& cameraStream,
                                videostreamclockcallback clock,
                                const XWalkMjpegHttpConfiguration& streamConfiguration);
            /** @brief Stops the listener and camera without throwing. */
            ~XWalkVideoStreaming() noexcept;

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            XWalkVideoStreaming(const XWalkVideoStreaming&) = delete;
            XWalkVideoStreaming(XWalkVideoStreaming&&) = delete;
            XWalkVideoStreaming& operator=(const XWalkVideoStreaming&) = delete;
            XWalkVideoStreaming& operator=(XWalkVideoStreaming&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Opens the camera and starts the configured HTTP listener.
             * @return True when both resources are active; otherwise false after cleanup.
             */
            agent::boolean start();
            /**
             * @brief Captures, publishes, and pumps one bounded stream iteration.
             * @return True when the camera and HTTP server remain usable; otherwise false.
             */
            agent::boolean step();
            /** @brief Idempotently closes clients, listener, queues, and camera. */
            void stop() noexcept;
            /**
             * @brief Returns the active listener port, or zero while stopped.
             * @return Listener port in host byte order, or zero.
             */
            agent::uint32 port() const noexcept;
            /**
             * @brief Reports whether camera and listener startup completed.
             * @return True only while both resources are retained.
             */
            agent::boolean started() const noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VIDEO_STREAMING_H */
