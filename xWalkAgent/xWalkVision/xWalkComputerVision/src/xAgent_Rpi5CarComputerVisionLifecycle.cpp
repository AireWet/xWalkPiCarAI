/******************************************************************************
 * @file        xAgent_Rpi5CarComputerVisionLifecycle.cpp
 * @brief       Implements computer-vision lifecycle and scheduling.
 *
 * @details
 * Validates the injected provider, starts and stops it, and preserves the
 * source example's 500-millisecond post-key delay with cancellation polling.
 *
 * @project     xWalk Firmware
 * @module      xWalkComputerVision
 *
 * @author      Joxy John
 * @date        2026-08-04
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

#include "xAgent_Rpi5CarComputerVision.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Binds one caller-owned provider and scheduling context.
 * @param[in,out] context Optional callback context that outlives this Agent.
 * @param[in] providerCallbacks Complete synchronous callback table.
 * @throws std::invalid_argument If any required callback is null.
 */
XWalkComputerVision::XWalkComputerVision(agent::contextpointer context,
    const XWalkComputerVisionCallbacks& providerCallbacks):
    callbackContext(context), callbacks(providerCallbacks)
{
    validateCallbacks(callbacks);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Stops an active provider without releasing caller-owned resources. */
XWalkComputerVision::~XWalkComputerVision() noexcept
{
    stop();
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates that the complete callback boundary is non-null.
 * @param[in] providerCallbacks Callback table to validate.
 * @throws std::invalid_argument If any required callback is null.
 */
void XWalkComputerVision::validateCallbacks(
    const XWalkComputerVisionCallbacks& providerCallbacks)
{
    if ((providerCallbacks.start == nullptr) ||
        (providerCallbacks.stop == nullptr) ||
        (providerCallbacks.capture == nullptr) ||
        (providerCallbacks.setColor == nullptr) ||
        (providerCallbacks.setFace == nullptr) ||
        (providerCallbacks.setQr == nullptr) ||
        (providerCallbacks.observe == nullptr) ||
        (providerCallbacks.delay == nullptr) ||
        (providerCallbacks.continueOperation == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Computer vision requires complete callbacks");
    }
}

/**
 * @brief Waits 500 milliseconds in cancellable 20-millisecond slices.
 * @return `true` after the complete wait or `false` after cancellation.
 */
agent::boolean XWalkComputerVision::waitAfterCommand() const
{
    constexpr agent::uint32 totalDelayMs{500U};
    constexpr agent::uint32 cancellationIntervalMs{20U};
    agent::uint32 remainingMs = totalDelayMs;
    while (remainingMs > 0U)
    {
        const agent::boolean operationRequested =
            callbacks.continueOperation(callbackContext);
        if (operationRequested == false)
        {
            return false;
        }
        const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ?
            remainingMs : cancellationIntervalMs;
        callbacks.delay(callbackContext, sliceMs);
        remainingMs -= sliceMs;
    }
    return callbacks.continueOperation(callbackContext);
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Starts camera acquisition and resets every retained detector mode.
 * @return `true` when the provider started; otherwise `false`.
 */
agent::boolean XWalkComputerVision::start()
{
    if (startedValue)
    {
        return true;
    }
    colorValue = XWalkComputerVisionColor::Close;
    faceEnabledValue = false;
    qrEnabledValue = false;
    lastQrData.clear();
    startedValue = callbacks.start(callbackContext);
    return startedValue;
}

/** @brief Disables detectors and stops the active provider. */
void XWalkComputerVision::stop() noexcept
{
    if (startedValue)
    {
        callbacks.stop(callbackContext);
    }
    colorValue = XWalkComputerVisionColor::Close;
    faceEnabledValue = false;
    qrEnabledValue = false;
    lastQrData.clear();
    startedValue = false;
}

/**
 * @brief Reports whether the provider is currently active.
 * @return `true` after successful start and before stop.
 */
agent::boolean XWalkComputerVision::started() const noexcept
{
    return startedValue;
}

} /* namespace xwalk::agent */
