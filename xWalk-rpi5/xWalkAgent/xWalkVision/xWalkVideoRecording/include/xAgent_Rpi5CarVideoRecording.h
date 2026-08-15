/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecording.h
 * @brief       Declares source-compatible interactive video recording.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VIDEO_RECORDING_H
#define XAGENT_RPI5CAR_VIDEO_RECORDING_H

#include "xAgent_Rpi5CarVideoRecordingTypes.h"

namespace xwalk::agent
{

    /** @brief Coordinates the key-driven lifecycle from `9.record_video.py`. */
    class XWalkVideoRecording final
    {
        private:
            agent::contextpointer callbackContext{nullptr};
            XWalkVideoRecordingCallbacks callbacks{};
            XWalkVideoRecordingState stateValue{XWalkVideoRecordingState::Stopped};
            agent::string videoPathValue{};
            agent::boolean startedValue{};

        protected:
            static void validateCallbacks(const XWalkVideoRecordingCallbacks& providerCallbacks);
            agent::boolean wait(agent::uint32 durationMs) const;

        public:
            /** @brief Binds one caller-owned provider and scheduling context. */
            XWalkVideoRecording(agent::contextpointer context, const XWalkVideoRecordingCallbacks& providerCallbacks);
            ~XWalkVideoRecording() noexcept;

            XWalkVideoRecording(const XWalkVideoRecording&) = delete;
            XWalkVideoRecording(XWalkVideoRecording&&) = delete;
            XWalkVideoRecording& operator=(const XWalkVideoRecording&) = delete;
            XWalkVideoRecording& operator=(XWalkVideoRecording&&) = delete;

            /** @brief Starts the camera and performs the cancellable 800 ms warm-up. */
            agent::boolean start();
            /** @brief Applies one case-insensitive source-compatible recording key. */
            XWalkVideoRecordingResult handleKey(agent::stringview keyText);
            /** @brief Finalizes an active AVI and closes the camera without throwing. */
            void stop() noexcept;
            /** @brief Returns the retained recording state. */
            XWalkVideoRecordingState state() const noexcept;
            /** @brief Reports whether the camera completed startup. */
            agent::boolean started() const noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VIDEO_RECORDING_H */
