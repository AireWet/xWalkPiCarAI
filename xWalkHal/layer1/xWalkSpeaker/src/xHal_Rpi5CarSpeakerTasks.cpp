/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerTasks.cpp
 * @brief       Implements speaker task progress, control, joining, and cleanup.
 *
 * @details
 * Provides task lookup, pause, resume, stop, listing, finished-worker reaping,
 * and bounded shutdown.
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

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Returns progress for one active playback task.
 *
 * @param[in] taskId
 * Identifier returned by `play()`.
 *
 * @return
 * Frame position, ratios, durations, and active-writing state.
 *
 * @throws std::invalid_argument
 * If no active task has the identifier.
 *
 */
XWalkSpeakerProgress XWalkSpeaker::getProgress(stringview taskId) {
  reapFinishedTasks();
  XWalkSpeakerProgress progress{};
  {
    const mutexlock lock(stateMutex);
    const size taskIndex = findTaskSlot(taskId);
    if (taskIndex == XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX) {
      XWALK_HAL_ERROR(XWALK_INVAL,
                      "Speaker playback task identifier is invalid");
    }

    const XWalkSpeakerTaskSlot &task = taskSlots[taskIndex];
    const size channelCount = static_cast<size>(task.audioData.channelCount);
    const size totalFrames = task.audioData.samples.size() / channelCount;
    const float64 positionFrames = static_cast<float64>(task.positionFrames);
    const float64 totalFrameValue = static_cast<float64>(totalFrames);
    const float64 sampleRateHz =
        static_cast<float64>(task.audioData.sampleRateHz);
    const float64 progressRatio =
        (totalFrames > 0U) ? (positionFrames / totalFrameValue) : 0.0;
    const float64 elapsedSeconds = positionFrames / sampleRateHz;
    const float64 totalSeconds = totalFrameValue / sampleRateHz;
    progress = {task.positionFrames, totalFrames,  progressRatio,
                elapsedSeconds,      totalSeconds, task.playing};
  }
  XWALK_HAL_TRACE_UID3(RPI .314, "Speaker task %.*s progress is %.3f",
                       static_cast<int32>(taskId.size()), taskId.data(),
                       progress.progressRatio);
  return progress;
}

/**
 * @brief Pauses one active playback task at its next chunk boundary.
 *
 * @param[in] taskId
 * Identifier returned by `play()`.
 *
 * @throws std::invalid_argument
 * If no active task has the identifier.
 *
 */
void XWalkSpeaker::pause(stringview taskId) {
  reapFinishedTasks();
  {
    const mutexlock lock(stateMutex);
    const size taskIndex = findTaskSlot(taskId);
    if (taskIndex == XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX) {
      XWALK_HAL_ERROR(XWALK_INVAL,
                      "Speaker playback task identifier is invalid");
    }
    taskSlots[taskIndex].pauseRequested = true;
    taskSlots[taskIndex].playing = false;
  }
  XWALK_HAL_TRACE_UID2(RPI .311, "Speaker playback task %.*s paused",
                       static_cast<int32>(taskId.size()), taskId.data());
}

/**
 * @brief Resumes one paused playback task.
 *
 * @param[in] taskId
 * Identifier returned by `play()`.
 *
 * @throws std::invalid_argument
 * If no active task has the identifier.
 *
 */
void XWalkSpeaker::resume(stringview taskId) {
  reapFinishedTasks();
  {
    const mutexlock lock(stateMutex);
    const size taskIndex = findTaskSlot(taskId);
    if (taskIndex == XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX) {
      XWALK_HAL_ERROR(XWALK_INVAL,
                      "Speaker playback task identifier is invalid");
    }
    taskSlots[taskIndex].pauseRequested = false;
    if (taskSlots[taskIndex].stream != nullptr) {
      taskSlots[taskIndex].playing = true;
    }
  }
  XWALK_HAL_TRACE_UID2(RPI .312, "Speaker playback task %.*s resumed",
                       static_cast<int32>(taskId.size()), taskId.data());
}

/**
 * @brief Stops, joins, and removes one playback task.
 *
 * @param[in] taskId
 * Identifier returned by `play()`.
 *
 * @return
 * `true` when an active task is stopped; otherwise `false`.
 *
 */
boolean XWalkSpeaker::stop(stringview taskId) {
  reapFinishedTasks();
  size taskIndex = XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX;
  {
    const mutexlock lock(stateMutex);
    taskIndex = findTaskSlot(taskId);
    if (taskIndex == XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX) {
      return false;
    }
    taskSlots[taskIndex].stopRequested = true;
    taskSlots[taskIndex].pauseRequested = false;
    taskSlots[taskIndex].joining = true;
  }

  const hal::boolean workerJoinable =
      static_cast<hal::boolean>(taskSlots[taskIndex].worker.joinable());
  if (workerJoinable) {
    taskSlots[taskIndex].worker.join();
  }
  {
    const mutexlock lock(stateMutex);
    clearTaskSlot(taskIndex);
  }
  XWALK_HAL_TRACE_UID2(RPI .313, "Speaker playback task %.*s stopped",
                       static_cast<int32>(taskId.size()), taskId.data());
  return true;
}

/**
 * @brief Lists identifiers of every active playback task.
 *
 * @return
 * Active task identifiers in bounded slot order.
 *
 */
stringvector XWalkSpeaker::listTasks() {
  reapFinishedTasks();
  stringvector identifiers{};
  {
    const mutexlock lock(stateMutex);
    for (const XWalkSpeakerTaskSlot &task : taskSlots) {
      if (task.occupied && !task.finished) {
        identifiers.push_back(task.identifier);
      }
    }
  }
  XWALK_HAL_TRACE_UID1(RPI .315, "Speaker listed %zu active playback task(s)",
                       identifiers.size());
  return identifiers;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Finds an occupied unfinished task while the state mutex is held.
 *
 * @param[in] taskId
 * Non-empty playback identifier to locate.
 *
 * @return
 * Matching slot index or `XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX`.
 *
 * @pre
 * The calling execution context owns `stateMutex`.
 */
size XWalkSpeaker::findTaskSlot(stringview taskId) const noexcept {
  for (size taskIndex = 0U; taskIndex < taskSlots.size(); ++taskIndex) {
    const XWalkSpeakerTaskSlot &task = taskSlots[taskIndex];
    if (task.occupied && !task.finished && (task.identifier == taskId)) {
      return taskIndex;
    }
  }
  return XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX;
}

/**
 * @brief Clears a joined task slot while the state mutex is held.
 *
 * @param[in] taskIndex
 * Valid slot index whose worker is not joinable.
 *
 * @pre
 * The calling execution context owns `stateMutex`.
 */
void XWalkSpeaker::clearTaskSlot(size taskIndex) noexcept {
  XWalkSpeakerTaskSlot &task = taskSlots[taskIndex];
  task.identifier.clear();
  task.audioData = {};
  task.worker = threadhandle{};
  task.stream = nullptr;
  task.positionFrames = 0U;
  task.occupied = false;
  task.finished = false;
  task.joining = false;
  task.stopRequested = false;
  task.pauseRequested = false;
  task.playing = false;
}

/**
 * @brief Joins and clears every worker that has already finished.
 *
 */
void XWalkSpeaker::reapFinishedTasks() {
  for (size taskIndex = 0U; taskIndex < taskSlots.size(); ++taskIndex) {
    boolean reapTask = false;
    {
      const mutexlock lock(stateMutex);
      XWalkSpeakerTaskSlot &task = taskSlots[taskIndex];
      if (task.occupied && task.finished && !task.joining) {
        task.joining = true;
        reapTask = true;
      }
    }
    if (reapTask) {
      const hal::boolean workerJoinable =
          static_cast<hal::boolean>(taskSlots[taskIndex].worker.joinable());
      if (workerJoinable) {
        taskSlots[taskIndex].worker.join();
      }
      const mutexlock lock(stateMutex);
      clearTaskSlot(taskIndex);
    }
  }
}

/**
 * @brief Requests, joins, and clears every retained playback task.
 *
 */
void XWalkSpeaker::stopAllTasks() {
  for (size taskIndex = 0U; taskIndex < taskSlots.size(); ++taskIndex) {
    boolean stopTask = false;
    {
      const mutexlock lock(stateMutex);
      XWalkSpeakerTaskSlot &task = taskSlots[taskIndex];
      if (task.occupied && !task.joining) {
        task.stopRequested = true;
        task.pauseRequested = false;
        task.joining = true;
        stopTask = true;
      }
    }
    if (stopTask) {
      const hal::boolean workerJoinable =
          static_cast<hal::boolean>(taskSlots[taskIndex].worker.joinable());
      if (workerJoinable) {
        taskSlots[taskIndex].worker.join();
      }
      const mutexlock lock(stateMutex);
      clearTaskSlot(taskIndex);
    }
  }
}

} /* namespace xwalk::hal */
