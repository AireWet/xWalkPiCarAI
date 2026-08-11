/******************************************************************************
 * @file        xHal_Rpi5CarUserButton.cpp
 * @brief       Implements user-button monitoring and callback dispatch.
 *
 * @details
 * Polls active-low GPIO state, records monotonic press timing, recognizes short
 * and long presses, and synchronizes shared state.
 *
 * @project     xWalk Firmware
 * @module      xWalkUserButton
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

#include "xHal_Rpi5CarUserButton.h"

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
 * @brief Starts monitoring when no worker is currently running.
 *
 * @post
 * `isRunning()` returns `true` after successful worker creation.
 *
 * @throws std::system_error
 * If the monitoring thread cannot be created.
 *
 */
void XWalkUserButton::start() {
  const hal::boolean monitorAlreadyRunning =
      static_cast<hal::boolean>(monitorRunning.load());
  if (monitorAlreadyRunning) {
    return;
  }

  XWALK_HAL_TRACE_UID0(RPI .225, "User-button monitoring start requested");
  stopWorker();
  monitorRunning.store(true);
  monitorThread = threadhandle(&XWalkUserButton::monitorLoop, this);
}

/**
 * @brief Stops and joins the monitoring worker.
 *
 * @post
 * `isRunning()` returns `false` and no joinable worker remains.
 *
 */
void XWalkUserButton::stop() {
  stopWorker();
  XWALK_HAL_TRACE_UID0(RPI .226, "User-button monitoring stopped");
}

/**
 * @brief Provides the Python-compatible control alias for `stop()`.
 *
 */
void XWalkUserButton::close() { stop(); }

/**
 * @brief Configures or clears the short-click callback and context.
 *
 * @param[in,out] context
 * Non-owning callback context; nullability is callback-specific.
 *
 * @param[in] callback
 * Nullable callback invoked after a short press is released.
 */
void XWalkUserButton::setOnClick(contextpointer context,
                                 userbuttoncallback callback) {
  const mutexlock lock(stateMutex);
  clickContext = context;
  clickCallback = callback;
}

/**
 * @brief Configures or clears the press callback and context.
 *
 * @param[in,out] context
 * Non-owning callback context; nullability is callback-specific.
 *
 * @param[in] callback
 * Nullable callback invoked when a press is recognized.
 */
void XWalkUserButton::setOnPress(contextpointer context,
                                 userbuttoncallback callback) {
  const mutexlock lock(stateMutex);
  pressContext = context;
  pressCallback = callback;
}

/**
 * @brief Configures or clears the release callback and context.
 *
 * @param[in,out] context
 * Non-owning callback context; nullability is callback-specific.
 *
 * @param[in] callback
 * Nullable callback invoked when a release is recognized.
 */
void XWalkUserButton::setOnRelease(contextpointer context,
                                   userbuttoncallback callback) {
  const mutexlock lock(stateMutex);
  releaseContext = context;
  releaseCallback = callback;
}

/**
 * @brief Configures or clears the press/release state callback and context.
 *
 * @param[in,out] context
 * Non-owning callback context; nullability is callback-specific.
 *
 * @param[in] callback
 * Nullable callback invoked with `true` for press and `false` for release.
 */
void XWalkUserButton::setOnPressReleased(contextpointer context,
                                         userbuttonstatecallback callback) {
  const mutexlock lock(stateMutex);
  stateContext = context;
  stateCallback = callback;
}

/**
 * @brief Configures or clears the long-press callback and shared threshold.
 *
 * @param[in,out] context
 * Non-owning callback context; nullability is callback-specific.
 *
 * @param[in] callback
 * Nullable callback invoked once for an armed long press.
 *
 * @param[in] durationSeconds
 * Finite threshold clamped to the inclusive range 2.0 to 5.0 seconds.
 *
 * @throws std::invalid_argument
 * If `durationSeconds` is not finite.
 */
void XWalkUserButton::setOnLongPress(contextpointer context,
                                     userbuttoncallback callback,
                                     float64 durationSeconds) {
  const float64 validatedDuration = validatedLongPressDuration(durationSeconds);
  const mutexlock lock(stateMutex);
  longPressContext = context;
  longPressCallback = callback;
  longPressDurationSecondsValue = validatedDuration;
}

/**
 * @brief Configures or clears the long-press-release callback and threshold.
 *
 * @param[in,out] context
 * Non-owning callback context; nullability is callback-specific.
 *
 * @param[in] callback
 * Nullable callback invoked when a triggered long press is released.
 *
 * @param[in] durationSeconds
 * Finite threshold clamped to the inclusive range 2.0 to 5.0 seconds.
 *
 * @throws std::invalid_argument
 * If `durationSeconds` is not finite.
 */
void XWalkUserButton::setOnLongPressReleased(contextpointer context,
                                             userbuttoncallback callback,
                                             float64 durationSeconds) {
  const float64 validatedDuration = validatedLongPressDuration(durationSeconds);
  const mutexlock lock(stateMutex);
  longReleaseContext = context;
  longReleaseCallback = callback;
  longPressDurationSecondsValue = validatedDuration;
}

/**
 * @brief Returns whether the button is recognized as pressed.
 *
 * @return
 * `true` between a recognized active-low press and release; otherwise `false`.
 */
boolean XWalkUserButton::state() const {
  const mutexlock lock(stateMutex);
  return pressedValue;
}

/**
 * @brief Provides the Python-compatible state alias for `state()`.
 *
 * @return
 * `true` while the button is recognized as pressed; otherwise `false`.
 */
boolean XWalkUserButton::isPressed() const { return state(); }

/**
 * @brief Returns the active or most recently completed press duration.
 *
 * @return
 * Elapsed duration in seconds.
 */
float64 XWalkUserButton::pressedForSeconds() const {
  const mutexlock lock(stateMutex);
  if (!pressedValue) {
    return pressedForSecondsValue;
  }

  const uint64 currentMicroseconds = common::monotonicMicroseconds();
  const uint64 elapsedMicroseconds =
      currentMicroseconds - pressedAtMicrosecondsValue;
  const float64 elapsedValue = static_cast<float64>(elapsedMicroseconds);
  return elapsedValue / XHAL_RPI5CAR_USER_BUTTON_MICROSECONDS_PER_SECOND;
}

/**
 * @brief Returns the shared long-press threshold.
 *
 * @return
 * Configured threshold in the inclusive range 2.0 to 5.0 seconds.
 */
float64 XWalkUserButton::longPressDurationSeconds() const {
  const mutexlock lock(stateMutex);
  return longPressDurationSecondsValue;
}

/**
 * @brief Returns whether the monitoring worker is requested to run.
 *
 * @return
 * `true` while monitoring remains requested; otherwise `false`.
 */
boolean XWalkUserButton::isRunning() const noexcept {
  return monitorRunning.load();
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Polls the GPIO and dispatches transitions until stopped.
 *
 * @warning
 * A GPIO or callback exception terminates the process because this worker does
 * not install an exception handler.
 */
void XWalkUserButton::monitorLoop() noexcept {
  boolean previousLevel = gpioObject->read();
  const hal::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const hal::boolean monitorShouldRun =
        static_cast<hal::boolean>(monitorRunning.load());
    if (monitorShouldRun == false) {
      break;
    }
    const boolean currentLevel = gpioObject->read();
    if (currentLevel != previousLevel) {
      if (currentLevel == XHAL_RPI5CAR_USER_BUTTON_PRESSED_LEVEL) {
        handlePress();
      } else {
        handleRelease();
      }
      previousLevel = currentLevel;
    }

    handleLongPress();
    common::sleepMilliseconds(XHAL_RPI5CAR_USER_BUTTON_POLL_INTERVAL_MS);
  }
  monitorRunning.store(false);
}

/**
 * @brief Records and dispatches one active-low press transition.
 */
void XWalkUserButton::handlePress() {
  userbuttoncallback selectedPressCallback = nullptr;
  contextpointer selectedPressContext = nullptr;
  userbuttonstatecallback selectedStateCallback = nullptr;
  contextpointer selectedStateContext = nullptr;
  {
    const mutexlock lock(stateMutex);
    if (pressedValue) {
      return;
    }

    pressedValue = true;
    pressedAtMicrosecondsValue = common::monotonicMicroseconds();
    longPressTriggeredValue = false;
    longPressArmedValue =
        (longPressCallback != nullptr) || (longReleaseCallback != nullptr);
    const float64 durationMicroseconds =
        longPressDurationSecondsValue *
        XHAL_RPI5CAR_USER_BUTTON_MICROSECONDS_PER_SECOND;
    activeLongPressDurationMicroseconds =
        static_cast<uint64>(durationMicroseconds);
    selectedPressCallback = pressCallback;
    selectedPressContext = pressContext;
    selectedStateCallback = stateCallback;
    selectedStateContext = stateContext;
  }

  if (selectedPressCallback != nullptr) {
    selectedPressCallback(selectedPressContext);
  }
  if (selectedStateCallback != nullptr) {
    selectedStateCallback(selectedStateContext, true);
  }
}

/**
 * @brief Records and dispatches one release transition.
 */
void XWalkUserButton::handleRelease() {
  userbuttoncallback selectedReleaseCallback = nullptr;
  contextpointer selectedReleaseContext = nullptr;
  userbuttonstatecallback selectedStateCallback = nullptr;
  contextpointer selectedStateContext = nullptr;
  userbuttoncallback selectedCompletionCallback = nullptr;
  contextpointer selectedCompletionContext = nullptr;
  {
    const mutexlock lock(stateMutex);
    if (!pressedValue) {
      return;
    }

    const uint64 releaseMicroseconds = common::monotonicMicroseconds();
    const uint64 elapsedMicroseconds =
        releaseMicroseconds - pressedAtMicrosecondsValue;
    const float64 elapsedValue = static_cast<float64>(elapsedMicroseconds);
    pressedForSecondsValue =
        elapsedValue / XHAL_RPI5CAR_USER_BUTTON_MICROSECONDS_PER_SECOND;
    pressedValue = false;
    selectedReleaseCallback = releaseCallback;
    selectedReleaseContext = releaseContext;
    selectedStateCallback = stateCallback;
    selectedStateContext = stateContext;
    if (longPressTriggeredValue) {
      selectedCompletionCallback = longReleaseCallback;
      selectedCompletionContext = longReleaseContext;
    } else {
      selectedCompletionCallback = clickCallback;
      selectedCompletionContext = clickContext;
    }
    longPressArmedValue = false;
  }

  if (selectedReleaseCallback != nullptr) {
    selectedReleaseCallback(selectedReleaseContext);
  }
  if (selectedStateCallback != nullptr) {
    selectedStateCallback(selectedStateContext, false);
  }
  if (selectedCompletionCallback != nullptr) {
    selectedCompletionCallback(selectedCompletionContext);
  }
}

/**
 * @brief Triggers an armed long press once its threshold is reached.
 */
void XWalkUserButton::handleLongPress() {
  userbuttoncallback selectedCallback = nullptr;
  contextpointer selectedContext = nullptr;
  {
    const mutexlock lock(stateMutex);
    if ((!pressedValue) || (!longPressArmedValue) || longPressTriggeredValue) {
      return;
    }

    const uint64 currentMicroseconds = common::monotonicMicroseconds();
    const uint64 elapsedMicroseconds =
        currentMicroseconds - pressedAtMicrosecondsValue;
    if (elapsedMicroseconds < activeLongPressDurationMicroseconds) {
      return;
    }

    longPressTriggeredValue = true;
    selectedCallback = longPressCallback;
    selectedContext = longPressContext;
  }

  if (selectedCallback != nullptr) {
    selectedCallback(selectedContext);
  }
}

/**
 * @brief Stops and joins the monitoring worker.
 */
void XWalkUserButton::stopWorker() {
  monitorRunning.store(false);
  const hal::boolean monitorThreadJoinable =
      static_cast<hal::boolean>(monitorThread.joinable());
  if (monitorThreadJoinable) {
    monitorThread.join();
  }
}

/**
 * @brief Clamps a finite long-press threshold to the supported range.
 *
 * @param[in] durationSeconds
 * Requested threshold in seconds.
 *
 * @return
 * Threshold clamped to the inclusive range 2.0 to 5.0 seconds.
 *
 * @throws std::invalid_argument
 * If the requested threshold is not finite.
 */
float64 XWalkUserButton::validatedLongPressDuration(float64 durationSeconds) {
  const hal::boolean durationSecondsNotFinite =
      static_cast<hal::boolean>(!XHAL_IS_FINITE(durationSeconds));
  if (durationSecondsNotFinite) {
    XWALK_HAL_ERROR(XWALK_INVAL,
                    "User button long-press duration must be finite");
  }
  if (durationSeconds < XHAL_RPI5CAR_USER_BUTTON_MIN_LONG_PRESS_SECONDS) {
    return XHAL_RPI5CAR_USER_BUTTON_MIN_LONG_PRESS_SECONDS;
  }
  if (durationSeconds > XHAL_RPI5CAR_USER_BUTTON_MAX_LONG_PRESS_SECONDS) {
    return XHAL_RPI5CAR_USER_BUTTON_MAX_LONG_PRESS_SECONDS;
  }
  return durationSeconds;
}

} /* namespace xwalk::hal */
