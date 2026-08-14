/******************************************************************************
 * @file        xHal_Rpi5CarGpioLinux.cpp
 * @brief       Implements Linux GPIO line and edge-event operations.
 *
 * @details
 * Uses the Linux GPIO character-device version-one ABI for line claims, digital
 *I/O, and edge polling.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Linux Backend
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

#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarLinuxHeaders.h"
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
 * @brief Converts a mode and pull setting to Linux GPIO handle flags.
 *
 * @param[in] mode
 * Requested input or output mode.
 *
 * @param[in] pull
 * Requested internal bias.
 *
 * @return
 * Linux GPIO handle flag bit mask.
 */
uint32 XWalkGpioLinux::createHandleFlags(XWalkGpioMode mode,
                                         XWalkGpioPull pull) noexcept {
  uint32 flags = (mode == XWalkGpioMode::Input)
                     ? static_cast<uint32>(GPIOHANDLE_REQUEST_INPUT)
                     : static_cast<uint32>(GPIOHANDLE_REQUEST_OUTPUT);
  if (pull == XWalkGpioPull::Up) {
    flags |= static_cast<uint32>(GPIOHANDLE_REQUEST_BIAS_PULL_UP);
  } else if (pull == XWalkGpioPull::Down) {
    flags |= static_cast<uint32>(GPIOHANDLE_REQUEST_BIAS_PULL_DOWN);
  } else {
    flags |= static_cast<uint32>(GPIOHANDLE_REQUEST_BIAS_DISABLE);
  }
  return flags;
}

/**
 * @brief Converts an edge selection to Linux GPIO event flags.
 *
 * @param[in] edge
 * Requested rising, falling, or combined edge selection.
 *
 * @return
 * Linux GPIO event flag bit mask.
 */
uint32 XWalkGpioLinux::createEventFlags(XWalkGpioEdge edge) noexcept {
  if (edge == XWalkGpioEdge::Rising) {
    return static_cast<uint32>(GPIOEVENT_REQUEST_RISING_EDGE);
  }
  if (edge == XWalkGpioEdge::Falling) {
    return static_cast<uint32>(GPIOEVENT_REQUEST_FALLING_EDGE);
  }
  return static_cast<uint32>(GPIOEVENT_REQUEST_BOTH_EDGES);
}

/**
 * @brief Validates that the requested pin is assigned to this backend.
 *
 * @param[in] pin
 * GPIO line offset expected to match `pinValue`.
 *
 * @throws std::runtime_error
 * If no line is assigned or the offset differs from the assigned pin.
 */
void XWalkGpioLinux::validateAssignedPin(uint8 pin) const {
  if ((!pinAssigned) || (pin != pinValue) || (lineDescriptor < 0)) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "GPIO line is not configured by this backend");
  }
}

/**
 * @brief Waits for edge events and invokes the registered application handler.
 *
 * @details
 * Kernel event timestamps are compared in nanoseconds so debounce decisions do
 * not depend on wall-clock changes.
 *
 * @warning
 * The application handler executes on this worker thread and must not throw.
 */
void XWalkGpioLinux::interruptLoop() {
  const hal::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const boolean operationMayContinue = stopRequested.load() == false;
    if (operationMayContinue == false) {
      break;
    }
    const int32 pollResult = deviceInterfaceValue.pollEvent(
        lineDescriptor, XHAL_RPI5CAR_GPIO_EVENT_POLL_MS);
    if (pollResult <= 0) {
      continue;
    }

    gpioevent_data eventData{};
    const int32 bytesRead = deviceInterfaceValue.readEvent(
        lineDescriptor, &eventData, sizeof(eventData));
    if (bytesRead != static_cast<int32>(sizeof(eventData))) {
      continue;
    }
    const uint64 eventNanoseconds = static_cast<uint64>(eventData.timestamp);
    const uint64 elapsedNanoseconds = eventNanoseconds - lastEventNanoseconds;
    if ((lastEventNanoseconds == 0U) ||
        (elapsedNanoseconds >= debounceNanoseconds)) {
      lastEventNanoseconds = eventNanoseconds;
      XWALK_HAL_TRACE_UID1(RPI .075, "Linux GPIO event accepted for line %u",
                           static_cast<uint32>(pinValue));
      interruptHandler(interruptContext);
    }
  }
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Claims and configures one Linux GPIO line.
 *
 * @param[in] pin
 * GPIO line offset in the range 0 through 255.
 *
 * @param[in] mode
 * Requested input or output direction.
 *
 * @param[in] pull
 * Requested internal bias.
 *
 * @param[in] initialValue
 * Initial physical output level; ignored for input mode.
 *
 * @throws std::runtime_error
 * If the Linux line request fails.
 */
void XWalkGpioLinux::configurePin(uint8 pin, XWalkGpioMode mode,
                                  XWalkGpioPull pull, boolean initialValue) {
  cancelInterrupt(pin);
  const mutexlock lock(mutex);
  releaseLine();

  gpiohandle_request request{};
  request.lineoffsets[0U] = pin;
  request.flags = createHandleFlags(mode, pull);
  request.default_values[0U] = initialValue ? 1U : 0U;
  request.consumer_label[0U] = 'x';
  request.consumer_label[1U] = 'W';
  request.consumer_label[2U] = 'a';
  request.consumer_label[3U] = 'l';
  request.consumer_label[4U] = 'k';
  request.lines = 1U;
  const boolean lineRequested =
      deviceInterfaceValue.requestLine(chipDescriptor, &request);
  const boolean lineRequestFailed = lineRequested == false;
  if (lineRequestFailed) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Linux GPIO line configuration failed");
  }
  lineDescriptor = request.fd;
  pinValue = pin;
  pinAssigned = true;
  pullValue = pull;
  XWALK_HAL_TRACE_UID4(RPI .069,
                       "Linux GPIO line %u configured with mode %u, pull %u, "
                       "and initial value %u",
                       static_cast<uint32>(pin), static_cast<uint32>(mode),
                       static_cast<uint32>(pull),
                       static_cast<uint32>(initialValue));
}

/**
 * @brief Samples the configured physical GPIO line.
 *
 * @param[in] pin
 * GPIO line offset expected to match this backend's claimed line.
 *
 * @return
 * `true` when the physical line is high; otherwise `false`.
 *
 * @throws std::runtime_error
 * If the pin is not configured or the Linux read operation fails.
 */
boolean XWalkGpioLinux::readPin(uint8 pin) {
  const mutexlock lock(mutex);
  validateAssignedPin(pin);
  gpiohandle_data data{};
  const boolean lineRead = deviceInterfaceValue.readLine(lineDescriptor, &data);
  const boolean lineReadFailed = lineRead == false;
  if (lineReadFailed) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Linux GPIO read failed");
  }
  const boolean value = data.values[0U] != 0U;
  XWALK_HAL_TRACE_UID2(RPI .070, "Linux GPIO line %u read value %u",
                       static_cast<uint32>(pin), static_cast<uint32>(value));
  return value;
}

/**
 * @brief Drives the configured physical GPIO line.
 *
 * @param[in] pin
 * GPIO line offset expected to match this backend's claimed line.
 *
 * @param[in] value
 * Physical output level to drive.
 *
 * @throws std::runtime_error
 * If the pin is not configured or the Linux write operation fails.
 */
void XWalkGpioLinux::writePin(uint8 pin, boolean value) {
  const mutexlock lock(mutex);
  validateAssignedPin(pin);
  gpiohandle_data data{};
  data.values[0U] = value ? 1U : 0U;
  const boolean lineWritten =
      deviceInterfaceValue.writeLine(lineDescriptor, &data);
  const boolean lineWriteFailed = lineWritten == false;
  if (lineWriteFailed) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Linux GPIO write failed");
  }
  XWALK_HAL_TRACE_UID2(RPI .071, "Linux GPIO line %u wrote value %u",
                       static_cast<uint32>(pin), static_cast<uint32>(value));
}

/**
 * @brief Registers a debounced Linux GPIO edge handler.
 *
 * @param[in] pin
 * GPIO line offset associated with this backend.
 *
 * @param[in] edge
 * Signal transition to observe.
 *
 * @param[in] debounceMs
 * Minimum accepted edge interval in milliseconds.
 *
 * @param[in,out] handlerContext
 * Non-owning application context forwarded to `handler`.
 *
 * @param[in] handler
 * Non-null application handler invoked from the event worker.
 *
 * @throws std::invalid_argument
 * If `handler` is null.
 *
 * @throws std::runtime_error
 * If the Linux event-line request fails.
 */
void XWalkGpioLinux::registerInterrupt(uint8 pin, XWalkGpioEdge edge,
                                       uint32 debounceMs,
                                       contextpointer handlerContext,
                                       gpiointerrupthandler handler) {
  if (handler == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "GPIO interrupt handler must not be null");
  }
  cancelInterrupt(pin);
  const mutexlock lock(mutex);
  releaseLine();

  gpioevent_request request{};
  request.lineoffset = pin;
  request.handleflags = createHandleFlags(XWalkGpioMode::Input, pullValue);
  request.eventflags = createEventFlags(edge);
  request.consumer_label[0U] = 'x';
  request.consumer_label[1U] = 'W';
  request.consumer_label[2U] = 'a';
  request.consumer_label[3U] = 'l';
  request.consumer_label[4U] = 'k';
  const boolean eventRequested =
      deviceInterfaceValue.requestEvent(chipDescriptor, &request);
  const boolean interruptRegistrationFailed = eventRequested == false;
  if (interruptRegistrationFailed) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Linux GPIO interrupt registration failed");
  }

  lineDescriptor = request.fd;
  pinValue = pin;
  pinAssigned = true;
  interruptContext = handlerContext;
  interruptHandler = handler;
  const uint64 debounceMilliseconds = static_cast<uint64>(debounceMs);
  debounceNanoseconds = debounceMilliseconds * 1'000'000U;
  lastEventNanoseconds = 0U;
  stopRequested.store(false);
  eventThread = threadhandle(&XWalkGpioLinux::interruptLoop, this);
  XWALK_HAL_TRACE_UID3(RPI .072,
                       "Linux GPIO interrupt registered for line %u with edge "
                       "%u and %u milliseconds debounce",
                       static_cast<uint32>(pin), static_cast<uint32>(edge),
                       debounceMs);
}

/**
 * @brief Cancels event dispatch for the specified GPIO line.
 *
 * @param[in] pin
 * GPIO line offset associated with this backend.
 *
 * @post
 * The event worker is joined and the event descriptor is closed.
 *
 * @throws std::runtime_error
 * If `pin` differs from the line assigned to this backend.
 */
void XWalkGpioLinux::cancelInterrupt(uint8 pin) {
  if (pinAssigned && (pin != pinValue)) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "GPIO backend is assigned to a different pin");
  }
  stopRequested.store(true);
  const boolean eventThreadJoinable = eventThread.joinable();
  if (eventThreadJoinable) {
    eventThread.join();
    const mutexlock lock(mutex);
    releaseLine();
    interruptContext = nullptr;
    interruptHandler = nullptr;
    XWALK_HAL_TRACE_UID1(RPI .073, "Linux GPIO interrupt cancelled for line %u",
                         static_cast<uint32>(pin));
  }
}

} /* namespace xwalk::hal */
