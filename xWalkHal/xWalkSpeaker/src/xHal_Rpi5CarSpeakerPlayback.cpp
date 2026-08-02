/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerPlayback.cpp
 * @brief       Implements audio-file validation, decoding, and task creation.
 *
 * @details
 * Selects the Python-compatible decoder family, validates decoded interleaved
 * audio, allocates one bounded slot, and starts its playback worker.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeaker.h"

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
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Decodes an audio file and starts background playback.
 *
 * @param[in] filePath
 * Existing regular WAV, FLAC, OGG, MP3, M4A, AAC, or WMA file.
 *
 * @return
 * Non-empty identifier unique among active playback tasks.
 *
 * @throws std::runtime_error
 * If the file is absent, not regular, decoded data is invalid, no task slot is
 * available, or the backend returns a duplicate identifier.
 *
 * @throws std::invalid_argument
 * If the path is empty or has an unsupported extension.
 *
 * @throws filesystemerror
 * If the filesystem cannot inspect the requested path.
 *
 * @throws std::system_error
 * If the playback worker cannot be created.
 */
string XWalkSpeaker::play(stringview filePath)
{
    reapFinishedTasks();
    if (filePath.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("Speaker audio-file path must not be empty");
    }

    const filesystempath audioPath{string(filePath)};
    if (!filesystemEntryExists(audioPath) || !isRegularFile(audioPath))
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker audio-file path must identify an existing regular file");
    }

    const XWalkSpeakerAudioHandler handler = audioHandler(filePath);
    XWalkSpeakerAudioData audioData = callbacks.decodeAudio(backendContext, filePath, handler);
    validateAudioData(audioData);
    string taskId = callbacks.createTaskId(backendContext);
    if (taskId.empty())
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker backend returned an empty task identifier");
    }

    size selectedIndex = XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX;
    const mutexlock lock(stateMutex);
    if (findTaskSlot(taskId) != XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker backend returned a duplicate task identifier");
    }
    for (size taskIndex = 0U; taskIndex < taskSlots.size(); ++taskIndex)
    {
        if (!taskSlots[taskIndex].occupied)
        {
            selectedIndex = taskIndex;
            break;
        }
    }
    if (selectedIndex == XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker has no available playback task slot");
    }

    XWalkSpeakerTaskSlot& task = taskSlots[selectedIndex];
    task.identifier = taskId;
    task.audioData = std::move(audioData);
    task.positionFrames = 0U;
    task.occupied = true;
    task.finished = false;
    task.joining = false;
    task.stopRequested = false;
    task.pauseRequested = false;
    task.playing = false;
    task.worker = threadhandle(&XWalkSpeaker::playbackLoop, this, selectedIndex);
    return taskId;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Selects a Python-compatible decoder from a filename extension.
 *
 * @param[in] filePath
 * Non-empty audio-file path ending in a supported extension.
 *
 * @return
 * SoundFile for WAV, FLAC, or OGG; otherwise Librosa for MP3, M4A, AAC, or WMA.
 *
 * @throws std::invalid_argument
 * If the extension is absent or unsupported.
 */
XWalkSpeakerAudioHandler XWalkSpeaker::audioHandler(stringview filePath)
{
    const size extensionSeparator = filePath.find_last_of('.');
    const size directorySeparator = filePath.find_last_of('/');
    const boolean extensionMissing =
        (extensionSeparator == stringview::npos) ||
        ((directorySeparator != stringview::npos) &&
            (extensionSeparator < directorySeparator));
    if (extensionMissing || ((extensionSeparator + 1U) >= filePath.size()))
    {
        XHAL_THROW_INVALID_ARGUMENT("Speaker audio file requires a supported extension");
    }

    string extension{filePath.substr(extensionSeparator + 1U)};
    for (char& character : extension)
    {
        if ((character >= 'A') && (character <= 'Z'))
        {
            character = static_cast<char>(character + ('a' - 'A'));
        }
    }
    if ((extension == "wav") || (extension == "flac") || (extension == "ogg"))
    {
        return XWalkSpeakerAudioHandler::SoundFile;
    }
    if ((extension == "mp3") || (extension == "m4a") || (extension == "aac") ||
        (extension == "wma"))
    {
        return XWalkSpeakerAudioHandler::Librosa;
    }
    XHAL_THROW_INVALID_ARGUMENT("Speaker audio-file extension is unsupported");
}

/**
 * @brief Validates decoded sample rate, channels, and frame alignment.
 *
 * @param[in] audioData
 * Decoded interleaved audio returned by the backend.
 *
 * @throws std::runtime_error
 * If metadata is zero, samples do not form complete frames, or a sample is non-finite.
 */
void XWalkSpeaker::validateAudioData(const XWalkSpeakerAudioData& audioData)
{
    if ((audioData.sampleRateHz == 0U) || (audioData.channelCount == 0U))
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker backend returned zero-valued audio metadata");
    }
    const size channelCount = static_cast<size>(audioData.channelCount);
    if ((audioData.samples.size() % channelCount) != 0U)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker backend returned incomplete interleaved frames");
    }
    for (const float64 sample : audioData.samples)
    {
        if (!XHAL_IS_FINITE(sample))
        {
            XHAL_THROW_RUNTIME_ERROR("Speaker backend returned a non-finite audio sample");
        }
    }
}

} /* namespace xwalk::hal */
