/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerAlsaLifecycle.cpp
 * @brief       Implements Speaker ALSA adapter lifecycle and callback binding.
 *
 * @details
 * Validates the shared dependency, decoder seam, and configured volume before
 * publishing the complete callback table used by `XWalkSpeaker`.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker ALSA Adapter
 *
 * @author      Joxy John
 * @date        2026-08-01
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

#include "xHal_Rpi5CarSpeakerAlsa.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs an adapter with the built-in bounded PCM WAVE decoder.
 *
 * @param[in,out] sharedAudioBackend
 * Caller-owned shared ALSA backend that must outlive this adapter.
 *
 * @param[in] playbackVolumePercent
 * Mixer volume from zero through one hundred percent applied on enable.
 *
 * @throws std::out_of_range
 * If the volume exceeds one hundred percent.
 */
XWalkSpeakerAlsa::XWalkSpeakerAlsa(XWalkAudioAlsa& sharedAudioBackend,
    uint8 playbackVolumePercent):
    XWalkSpeakerAlsa(sharedAudioBackend, nullptr, systemOperations(),
        playbackVolumePercent)
{
}

/**
 * @brief Constructs an adapter with an injected optional decoder.
 *
 * @param[in,out] sharedAudioBackend
 * Caller-owned shared ALSA backend that must outlive this adapter.
 *
 * @param[in,out] context
 * Nullable non-owning decoder context that must outlive this adapter.
 *
 * @param[in] backendOperations
 * Operation table containing one non-null bounded decoder callback.
 *
 * @param[in] playbackVolumePercent
 * Mixer volume from zero through one hundred percent applied on enable.
 *
 * @throws std::invalid_argument
 * If the decoder callback is null.
 *
 * @throws std::out_of_range
 * If the volume exceeds one hundred percent.
 */
XWalkSpeakerAlsa::XWalkSpeakerAlsa(XWalkAudioAlsa& sharedAudioBackend,
    contextpointer context, const XWalkSpeakerAlsaOperations& backendOperations,
    uint8 playbackVolumePercent):
    audioBackend(&sharedAudioBackend), decoderContext(context),
    operations(backendOperations), playbackVolumePercentValue(playbackVolumePercent)
{
    if (operations.decodeAudio == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Speaker ALSA adapter requires a decoder callback");
    }
    if (playbackVolumePercentValue > 100U)
    {
        XHAL_THROW_OUT_OF_RANGE("Speaker ALSA volume exceeds one hundred percent");
    }
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the adapter without releasing its caller-owned audio backend.
 */
XWalkSpeakerAlsa::~XWalkSpeakerAlsa() = default;

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Returns the complete callback table for this adapter context.
 *
 * @return
 * Seven non-null callbacks suitable for `XWalkSpeaker` construction.
 */
XWalkSpeakerCallbacks XWalkSpeakerAlsa::callbacks() const noexcept
{
    return {&enableOutput, &disableOutput, &decodeAudio, &openStream,
        &writeStream, &closeStream, &createTaskId};
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Converts a callback context to its required adapter.
 *
 * @param[in,out] context
 * Non-null adapter context supplied to `XWalkSpeaker`.
 *
 * @return
 * Adapter referenced by the callback context.
 *
 * @throws std::invalid_argument
 * If the context is null.
 */
XWalkSpeakerAlsa& XWalkSpeakerAlsa::adapter(contextpointer context)
{
    if (context == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Speaker ALSA callback context must not be null");
    }
    return *static_cast<XWalkSpeakerAlsa*>(context);
}

} /* namespace xwalk::hal */
