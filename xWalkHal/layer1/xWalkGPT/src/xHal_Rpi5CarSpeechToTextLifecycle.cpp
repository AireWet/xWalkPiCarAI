/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextLifecycle.cpp
 * @brief       Implements speech-recognition validation and lifecycle behavior.
 *
 * @details
 * Validates a complete application backend, retains its non-owning context,
 * and requests cancellation during destruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarSpeechToText.h"
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
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates that every required backend callback is non-null.
 *
 * @param[in] backendCallbacks
 * Callback table to validate before storing or invoking it.
 *
 * @throws std::invalid_argument
 * If any callback is null.
 */
void XWalkSpeechToText::validateCallbacks(
    const XWalkSpeechToTextCallbacks &backendCallbacks) {
  if ((backendCallbacks.ready == nullptr) ||
      (backendCallbacks.listen == nullptr) ||
      (backendCallbacks.transcribeFile == nullptr) ||
      (backendCallbacks.stop == nullptr)) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Speech-to-text backend requires every callback");
  }
}

/**
 * @brief Validates one bounded microphone-recognition interval.
 *
 * @param[in] timeoutMs
 * Requested interval in milliseconds.
 *
 * @throws std::out_of_range
 * If the interval is zero or greater than the configured maximum.
 */
void XWalkSpeechToText::validateTimeout(uint32 timeoutMs) {
  if ((timeoutMs == 0U) ||
      (timeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS)) {
    XWALK_HAL_ERROR(XWALK_RANGE,
                    "Speech-to-text timeout is outside its supported range");
  }
}

/**
 * @brief Validates one audio-file path before backend dispatch.
 *
 * @param[in] filePath
 * Path view that must contain at least one character.
 *
 * @throws std::invalid_argument
 * If `filePath` is empty.
 */
void XWalkSpeechToText::validateFilePath(stringview filePath) {
  const hal::boolean filePathEmpty =
      static_cast<hal::boolean>(filePath.empty());
  if (filePathEmpty) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "Speech-to-text audio-file path must not be empty");
  }
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a speech coordinator from a complete backend table.
 *
 * @param[in,out] context
 * Nullable non-owning backend context. A non-null object must outlive this
 * coordinator, and null requires explicit support from all callbacks.
 *
 * @param[in] backendCallbacks
 * Complete callback table copied into this coordinator.
 *
 * @throws std::invalid_argument
 * If any required callback is null.
 */
XWalkSpeechToText::XWalkSpeechToText(
    contextpointer context, const XWalkSpeechToTextCallbacks &backendCallbacks)
    : backendContextPointer(context), callbacks(backendCallbacks) {
  validateCallbacks(callbacks);
  XWALK_HAL_TRACE_UID0(RPI .357, "Speech-to-text coordinator constructed");
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Requests recognition stop and releases no backend ownership.
 *
 * @warning
 * The stop callback must not throw because the destructor cannot report a
 * backend shutdown failure.
 */
XWalkSpeechToText::~XWalkSpeechToText() { stop(); }

} /* namespace xwalk::hal */
