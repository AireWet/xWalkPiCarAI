/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerTypes.h
 * @brief       Declares speaker audio, progress, task, and backend types.
 *
 * @details
 * Defines bounded playback state and the callback boundary used to decode
 * supported audio files and write floating-point frames to platform streams.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_SPEAKER_TYPES_H
#define XHAL_RPI5CAR_SPEAKER_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/** @brief Selects the Python-compatible decoder family for an audio file. */
enum class XWalkSpeakerAudioHandler : uint8
{
    SoundFile = 0U, /**< Decoder used for WAV, FLAC, and OGG files. */
    Librosa = 1U    /**< Decoder used for MP3, M4A, AAC, and WMA files. */
};

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Contains decoded interleaved floating-point audio frames. */
struct XWalkSpeakerAudioData
{
    /** @brief Interleaved normalized samples ordered by frame and channel. */
    float64vector samples{};
    /** @brief Positive playback sample rate in Hertz. */
    uint32 sampleRateHz{};
    /** @brief Positive number of interleaved channels. */
    uint8 channelCount{};
};

/** @brief Reports one active speaker task's position and playback state. */
struct XWalkSpeakerProgress
{
    /** @brief Current zero-based playback position in complete audio frames. */
    size positionFrames{};
    /** @brief Total number of decoded audio frames. */
    size totalFrames{};
    /** @brief Completion ratio in the inclusive range 0.0 to 1.0. */
    float64 progressRatio{};
    /** @brief Current playback position in seconds. */
    float64 elapsedSeconds{};
    /** @brief Total decoded duration in seconds. */
    float64 totalSeconds{};
    /** @brief `true` while the task is actively writing rather than paused. */
    boolean isPlaying{};
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Opaque nullable platform stream handle returned by the audio backend. */
using speakerstreamhandle = contextpointer;

/**
 * @brief Callback that changes the physical speaker-enable state.
 *
 * @param[in,out] context
 * Non-owning backend context; nullability is backend-specific.
 */
using speakeroutputcallback = void (*)(contextpointer context);

/**
 * @brief Callback that decodes one supported audio file.
 *
 * @param[in,out] context
 * Non-owning backend context; nullability is backend-specific.
 *
 * @param[in] filePath
 * Existing regular-file path view valid only for the callback duration.
 *
 * @param[in] handler
 * Python-compatible decoder family selected from the filename extension.
 *
 * @return
 * Interleaved normalized samples with their sample rate and channel count.
 */
using speakeraudiodecodecallback = XWalkSpeakerAudioData (*)(contextpointer context,
    stringview filePath, XWalkSpeakerAudioHandler handler);

/**
 * @brief Callback that opens one platform output stream.
 *
 * @param[in,out] context
 * Non-owning backend context; nullability is backend-specific.
 *
 * @param[in] sampleRateHz
 * Positive playback sample rate in Hertz.
 *
 * @param[in] channelCount
 * Positive number of interleaved channels.
 *
 * @return
 * Non-null opaque stream handle owned by the backend until `closeStream`.
 */
using speakerstreamopencallback = speakerstreamhandle (*)(contextpointer context,
    uint32 sampleRateHz, uint8 channelCount);

/**
 * @brief Callback that writes a bounded range of decoded frames.
 *
 * @param[in,out] context
 * Non-owning backend context; nullability is backend-specific.
 *
 * @param[in,out] stream
 * Non-null backend stream returned by `openStream`.
 *
 * @param[in] audioData
 * Immutable decoded audio retained for the complete task lifetime.
 *
 * @param[in] firstFrame
 * Zero-based first frame to write.
 *
 * @param[in] frameCount
 * Positive number of consecutive frames to write.
 */
using speakerstreamwritecallback = void (*)(contextpointer context,
    speakerstreamhandle stream, const XWalkSpeakerAudioData& audioData,
    size firstFrame, size frameCount);

/**
 * @brief Callback that stops and closes one platform output stream.
 *
 * @param[in,out] context
 * Non-owning backend context; nullability is backend-specific.
 *
 * @param[in,out] stream
 * Non-null backend-owned stream handle that becomes invalid after the call.
 */
using speakerstreamclosecallback = void (*)(contextpointer context,
    speakerstreamhandle stream);

/**
 * @brief Callback that creates one playback-task identifier.
 *
 * @param[in,out] context
 * Non-owning backend context; nullability is backend-specific.
 *
 * @return
 * Non-empty identifier that must differ from every active task identifier.
 */
using speakertaskidcallback = string (*)(contextpointer context);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Contains the complete caller-owned speaker backend operation table. */
struct XWalkSpeakerCallbacks
{
    /** @brief Enables physical speaker output. */
    speakeroutputcallback enableOutput{nullptr};
    /** @brief Disables physical speaker output. */
    speakeroutputcallback disableOutput{nullptr};
    /** @brief Decodes one supported audio file. */
    speakeraudiodecodecallback decodeAudio{nullptr};
    /** @brief Opens one floating-point audio stream. */
    speakerstreamopencallback openStream{nullptr};
    /** @brief Writes one bounded range of decoded frames. */
    speakerstreamwritecallback writeStream{nullptr};
    /** @brief Stops and closes one opened stream. */
    speakerstreamclosecallback closeStream{nullptr};
    /** @brief Creates a unique playback-task identifier. */
    speakertaskidcallback createTaskId{nullptr};
};

/** @brief Stores one bounded speaker task and its joinable worker state. */
struct XWalkSpeakerTaskSlot
{
    /** @brief Identifier returned to the caller while this slot is occupied. */
    string identifier{};
    /** @brief Decoded audio retained until the worker has been joined. */
    XWalkSpeakerAudioData audioData{};
    /** @brief Joinable worker owned by the speaker controller. */
    threadhandle worker{};
    /** @brief Nullable backend-owned stream handle valid only while opened. */
    speakerstreamhandle stream{nullptr};
    /** @brief Current playback position in complete frames. */
    size positionFrames{};
    /** @brief `true` while this slot retains task state. */
    boolean occupied{};
    /** @brief `true` after the worker has completed all cleanup. */
    boolean finished{};
    /** @brief `true` while a controlling operation is joining the worker. */
    boolean joining{};
    /** @brief `true` when the worker must stop at its next state check. */
    boolean stopRequested{};
    /** @brief `true` when playback is intentionally paused. */
    boolean pauseRequested{};
    /** @brief `true` while decoded frames are actively being written. */
    boolean playing{};
};

/** @brief Fixed storage for the maximum supported concurrent speaker tasks. */
using speakertaskslots =
    fixedarray<XWalkSpeakerTaskSlot, XHAL_RPI5CAR_SPEAKER_MAXIMUM_TASK_COUNT>;

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEAKER_TYPES_H */
