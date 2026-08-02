/******************************************************************************
 * @file        xHal_Rpi5CarGpioLinux.cpp
 * @brief       Implements Linux GPIO line and edge-event operations.
 *
 * @details
 * Uses the Linux GPIO character-device version-one ABI for line claims, digital I/O, and edge polling.
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
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

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
uint32 XWalkGpioLinux::createHandleFlags(XWalkGpioMode mode, XWalkGpioPull pull) noexcept
{
    uint32 flags = (mode == XWalkGpioMode::Input) ? static_cast<uint32>(GPIOHANDLE_REQUEST_INPUT) :
        static_cast<uint32>(GPIOHANDLE_REQUEST_OUTPUT);
    if (pull == XWalkGpioPull::Up)
    {
        flags |= static_cast<uint32>(GPIOHANDLE_REQUEST_BIAS_PULL_UP);
    }
    else if (pull == XWalkGpioPull::Down)
    {
        flags |= static_cast<uint32>(GPIOHANDLE_REQUEST_BIAS_PULL_DOWN);
    }
    else
    {
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
uint32 XWalkGpioLinux::createEventFlags(XWalkGpioEdge edge) noexcept
{
    if (edge == XWalkGpioEdge::Rising)
    {
        return static_cast<uint32>(GPIOEVENT_REQUEST_RISING_EDGE);
    }
    if (edge == XWalkGpioEdge::Falling)
    {
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
void XWalkGpioLinux::validateAssignedPin(uint8 pin) const
{
    if ((!pinAssigned) || (pin != pinValue) || (lineDescriptor < 0))
    {
        XHAL_THROW_RUNTIME_ERROR("GPIO line is not configured by this backend");
    }
}

/**
 * @brief Waits for edge events and invokes the registered application handler.
 *
 * @details
 * Kernel event timestamps are compared in nanoseconds so debounce decisions do not
 * depend on wall-clock changes.
 *
 * @warning
 * The application handler executes on this worker thread and must not throw.
 */
void XWalkGpioLinux::interruptLoop()
{
    while (!stopRequested.load())
    {
        pollfd descriptorEvent{};
        descriptorEvent.fd = lineDescriptor;
        descriptorEvent.events = POLLIN;
        const int32 pollResult = ::poll(&descriptorEvent, 1U, XHAL_RPI5CAR_GPIO_EVENT_POLL_MS);
        if ((pollResult <= 0) || ((descriptorEvent.revents & POLLIN) == 0))
        {
            continue;
        }

        gpioevent_data eventData{};
        const ssize_t bytesRead = ::read(lineDescriptor, &eventData, sizeof(eventData));
        if (bytesRead != static_cast<ssize_t>(sizeof(eventData)))
        {
            continue;
        }
        const uint64 eventNanoseconds = static_cast<uint64>(eventData.timestamp);
        const uint64 elapsedNanoseconds = eventNanoseconds - lastEventNanoseconds;
        if ((lastEventNanoseconds == 0U) || (elapsedNanoseconds >= debounceNanoseconds))
        {
            lastEventNanoseconds = eventNanoseconds;
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
void XWalkGpioLinux::configurePin(uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull,
    boolean initialValue)
{
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
    if (::ioctl(chipDescriptor, GPIO_GET_LINEHANDLE_IOCTL, &request) < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("Linux GPIO line configuration failed");
    }
    lineDescriptor = request.fd;
    pinValue = pin;
    pinAssigned = true;
    pullValue = pull;
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
boolean XWalkGpioLinux::readPin(uint8 pin)
{
    const mutexlock lock(mutex);
    validateAssignedPin(pin);
    gpiohandle_data data{};
    if (::ioctl(lineDescriptor, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("Linux GPIO read failed");
    }
    return data.values[0U] != 0U;
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
void XWalkGpioLinux::writePin(uint8 pin, boolean value)
{
    const mutexlock lock(mutex);
    validateAssignedPin(pin);
    gpiohandle_data data{};
    data.values[0U] = value ? 1U : 0U;
    if (::ioctl(lineDescriptor, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("Linux GPIO write failed");
    }
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
void XWalkGpioLinux::registerInterrupt(uint8 pin, XWalkGpioEdge edge, uint32 debounceMs,
    contextpointer handlerContext, gpiointerrupthandler handler)
{
    if (handler == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("GPIO interrupt handler must not be null");
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
    if (::ioctl(chipDescriptor, GPIO_GET_LINEEVENT_IOCTL, &request) < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("Linux GPIO interrupt registration failed");
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
void XWalkGpioLinux::cancelInterrupt(uint8 pin)
{
    if (pinAssigned && (pin != pinValue))
    {
        XHAL_THROW_RUNTIME_ERROR("GPIO backend is assigned to a different pin");
    }
    stopRequested.store(true);
    if (eventThread.joinable())
    {
        eventThread.join();
        const mutexlock lock(mutex);
        releaseLine();
        interruptContext = nullptr;
        interruptHandler = nullptr;
    }
}

} /* namespace xwalk::hal */
