/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordThreadExampleLinux.cpp
 * @brief       Implements Linux composition for threaded Vosk wake detection.
 *
 * @details
 * Composes ALSA and Vosk dependencies, owns the bounded recognition worker,
 * detects the configured phrase, and guarantees worker joining on every exit.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskWakeWordThreadExampleLinux.h"

#include "xHal_Rpi5CarSpeechRecognizerVosk.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"

#include "xHal_Rpi5CarTrace.h"
#include <algorithm>
#include <cctype>
#include <iostream>

namespace xwalk::hal::example {

/** @brief Stops and joins any listener retained during exceptional cleanup. */
XWalkSttVoskWakeWordThreadExampleLinux::
    ~XWalkSttVoskWakeWordThreadExampleLinux() noexcept {
  stopRequested.store(true);
  if (speechToTextObject != nullptr) {
    try {
      speechToTextObject->stop();
    } catch (...) {
      workerFailed.store(true);
    }
  }
  const hal::boolean workerJoinable =
      static_cast<hal::boolean>(worker.joinable());
  if (workerJoinable) {
    worker.join();
  }
}

/** @brief Composes live dependencies and runs bounded detection cycles. */
void XWalkSttVoskWakeWordThreadExampleLinux::run(
    uint32 detectionCount, uint32 maximumPolls, uint32 listenTimeoutMs,
    stringview microphoneDevice, stringview libraryName, stringview modelPath) {
  if ((listenTimeoutMs == 0U) ||
      (listenTimeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS)) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "Wake-word listen timeout is outside its range");
  }
  XWalkSpeechRecognizerVosk recognizer(libraryName, modelPath);
  XWalkSpeechToTextAlsa alsa(microphoneDevice, &recognizer,
                             recognizer.operations());
  XWalkSpeechToText speechToText(&alsa, alsa.callbacks());
  speechToTextObject = &speechToText;
  listenTimeoutMsValue = listenTimeoutMs;
  const XWalkSttVoskWakeWordThreadExampleCallbacks exampleCallbacks{
      &startListening, &isWaked, &stopListening, &wait, &report};
  XWalkSttVoskWakeWordThreadExample example(this, exampleCallbacks);
  try {
    example.run(detectionCount, maximumPolls);
    speechToTextObject = nullptr;
  } catch (...) {
    stopRequested.store(true);
    speechToText.stop();
    const hal::boolean workerJoinable =
        static_cast<hal::boolean>(worker.joinable());
    if (workerJoinable) {
      worker.join();
    }
    speechToTextObject = nullptr;
    throw;
  }
}

/** @brief Resolves one live adapter and speech binding. */
XWalkSttVoskWakeWordThreadExampleLinux &
XWalkSttVoskWakeWordThreadExampleLinux::adapter(contextpointer context) {
  if (context == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Wake-word Linux context must not be null");
  }
  XWalkSttVoskWakeWordThreadExampleLinux &self =
      *static_cast<XWalkSttVoskWakeWordThreadExampleLinux *>(context);
  if (self.speechToTextObject == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Wake-word Linux adapter has no speech binding");
  }
  return self;
}

/** @brief Starts one listener after confirming the previous worker was joined.
 */
void XWalkSttVoskWakeWordThreadExampleLinux::startListening(
    contextpointer context) {
  XWalkSttVoskWakeWordThreadExampleLinux &self = adapter(context);
  const hal::boolean workerAlreadyRunning =
      static_cast<hal::boolean>(self.worker.joinable());
  if (workerAlreadyRunning) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Wake-word listener is already active");
  }
  self.wakeDetected.store(false);
  self.stopRequested.store(false);
  self.workerFailed.store(false);
  self.worker = threadhandle(
      &XWalkSttVoskWakeWordThreadExampleLinux::listenForWakeWord, &self);
}

/** @brief Reports detection or wakes the polling loop after worker failure. */
boolean
XWalkSttVoskWakeWordThreadExampleLinux::isWaked(contextpointer context) {
  XWalkSttVoskWakeWordThreadExampleLinux &self = adapter(context);
  return self.wakeDetected.load() || self.workerFailed.load();
}

/** @brief Cancels recognition, joins the worker, and reports worker failure. */
void XWalkSttVoskWakeWordThreadExampleLinux::stopListening(
    contextpointer context) {
  XWalkSttVoskWakeWordThreadExampleLinux &self = adapter(context);
  self.stopRequested.store(true);
  self.speechToTextObject->stop();
  const hal::boolean workerJoinable =
      static_cast<hal::boolean>(self.worker.joinable());
  if (workerJoinable) {
    self.worker.join();
  }
  const hal::boolean exchangeSucceeded =
      static_cast<hal::boolean>(self.workerFailed.exchange(false));
  if (exchangeSucceeded) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Wake-word recognition worker failed");
  }
}

/** @brief Waits for one three-second source polling interval. */
void XWalkSttVoskWakeWordThreadExampleLinux::wait(contextpointer context,
                                                  uint32 durationMilliseconds) {
  static_cast<void>(adapter(context));
  common::sleepMilliseconds(durationMilliseconds);
}

/** @brief Prints one source-compatible status line. */
void XWalkSttVoskWakeWordThreadExampleLinux::report(contextpointer context,
                                                    stringview message) {
  static_cast<void>(adapter(context));
  std::cout << message << '\n';
}

/** @brief Repeats bounded recognition slices until stopped or awakened. */
void XWalkSttVoskWakeWordThreadExampleLinux::listenForWakeWord() noexcept {
  try {
    const hal::boolean processingLoopRequested{true};
    while (processingLoopRequested) {
      const hal::boolean listeningMayContinue = static_cast<hal::boolean>(
          !stopRequested.load() && !wakeDetected.load());
      if (listeningMayContinue == false) {
        break;
      }
      const string transcript =
          speechToTextObject->listen(listenTimeoutMsValue);
      const hal::boolean wakeWordDetected =
          static_cast<hal::boolean>(containsWakeWord(transcript));
      if (wakeWordDetected) {
        wakeDetected.store(true);
      }
    }
  } catch (...) {
    workerFailed.store(true);
  }
}

/** @brief Performs case-insensitive phrase detection on one transcript. */
boolean XWalkSttVoskWakeWordThreadExampleLinux::containsWakeWord(
    stringview transcript) {
  string normalizedTranscript(transcript);
  std::transform(normalizedTranscript.begin(), normalizedTranscript.end(),
                 normalizedTranscript.begin(), [](char value) {
                   return static_cast<char>(
                       std::tolower(static_cast<unsigned char>(value)));
                 });
  return normalizedTranscript.find(XHAL_RPI5CAR_STT_VOSK_WAKE_WORD) !=
         string::npos;
}

} /* namespace xwalk::hal::example */
