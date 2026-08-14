/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingTypes.h
 * @brief       Declares interactive video-recording states and callbacks.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VIDEO_RECORDING_TYPES_H
#define XAGENT_RPI5CAR_VIDEO_RECORDING_TYPES_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::agent
{

/** @brief Identifies the retained upstream recording state. */
enum class XWalkVideoRecordingState : agent::uint8
{
    /** @brief Camera is active without an open recording. */
    Stopped = 0U,
    /** @brief Captured frames are being written. */
    Recording,
    /** @brief The AVI remains open while frame writes are suspended. */
    Paused
};

/** @brief Identifies the result of one interactive key. */
enum class XWalkVideoRecordingEvent : agent::uint8
{
    /** @brief The key caused no source-compatible transition. */
    Ignored = 0U,
    /** @brief A new timestamped AVI was opened. */
    Started,
    /** @brief Recording was paused. */
    Paused,
    /** @brief Recording continued after a pause. */
    Continued,
    /** @brief The active AVI was finalized. */
    Stopped,
    /** @brief Cancellation interrupted the post-key wait. */
    Cancelled
};

using videorecordingstartcallback = agent::boolean (*)(agent::contextpointer context);
using videorecordingstopcallback = void (*)(agent::contextpointer context) noexcept;
using videorecordingbegincallback = agent::string (*)(agent::contextpointer context,
    agent::stringview recordingName);
using videorecordingcontrolcallback = void (*)(agent::contextpointer context);
using videorecordingstoprecordingcallback = void (*)(
    agent::contextpointer context) noexcept;
using videorecordingdelaycallback = void (*)(agent::contextpointer context,
    agent::uint32 durationMs);
using videorecordingcontinuecallback = agent::boolean (*)(agent::contextpointer context);
using videorecordingtimestampcallback = agent::string (*)(agent::contextpointer context);

/** @brief Groups the caller-owned camera, encoder, timing, and naming boundary. */
struct XWalkVideoRecordingCallbacks
{
    videorecordingstartcallback startCamera{nullptr};
    videorecordingstopcallback stopCamera{nullptr};
    videorecordingbegincallback beginRecording{nullptr};
    videorecordingcontrolcallback pauseRecording{nullptr};
    videorecordingcontrolcallback continueRecording{nullptr};
    videorecordingstoprecordingcallback stopRecording{nullptr};
    videorecordingdelaycallback delay{nullptr};
    videorecordingcontinuecallback continueOperation{nullptr};
    videorecordingtimestampcallback timestamp{nullptr};
};

/** @brief Reports one key transition, retained state, and relevant AVI path. */
struct XWalkVideoRecordingResult
{
    XWalkVideoRecordingEvent event{XWalkVideoRecordingEvent::Ignored};
    XWalkVideoRecordingState state{XWalkVideoRecordingState::Stopped};
    agent::string videoPath{};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VIDEO_RECORDING_TYPES_H */
