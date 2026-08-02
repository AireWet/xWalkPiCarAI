/******************************************************************************
 * @file        xAgent_Rpi5CarControllerLifecycle.cpp
 * @brief       Implements PiCar-X CLI lifecycle and callback forwarding.
 *
 * @details
 * Binds caller-owned dependencies and validates the synchronous platform boundary.
 *
 * @project     xWalk Firmware
 * @module      xWalkController
 *
 * @author      Joxy John
 * @date        2026-07-31
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
#include "xAgent_Rpi5CarController.h"
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
 * @brief Constructs a CLI around one caller-owned PiCar-X coordinator.
 * @param[in] picarx Coordinator that must outlive this CLI.
 * @param[in,out] context Optional platform context that must outlive this CLI when non-null.
 * @param[in] backendCallbacks Complete non-null synchronous callback table.
 */
XWalkController::XWalkController(XWalkPicarx& picarx, hal::contextpointer context,
    const XWalkControllerCallbacks& backendCallbacks)
    : picarxObject(&picarx), callbackContext(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
}

/**
 * @brief Constructs a CLI with foreground line-tracking command support.
 * @param[in] picarx Coordinator that must outlive this CLI.
 * @param[in] lineTracking Line-tracking coordinator that must outlive this CLI.
 * @param[in,out] context Optional platform context that must outlive this CLI when non-null.
 * @param[in] backendCallbacks Complete non-null synchronous callback table.
 */
XWalkController::XWalkController(XWalkPicarx& picarx, XWalkLineTracking& lineTracking,
    hal::contextpointer context, const XWalkControllerCallbacks& backendCallbacks)
    : picarxObject(&picarx), lineTrackingObject(&lineTracking),
      callbackContext(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
}

/**
 * @brief Constructs a CLI with named self-drive action support.
 * @param[in] picarx Coordinator that must outlive this CLI.
 * @param[in] selfDrive Self-drive coordinator that must outlive this CLI.
 * @param[in,out] context Optional platform context that must outlive this CLI when non-null.
 * @param[in] backendCallbacks Complete non-null synchronous callback table.
 */
XWalkController::XWalkController(XWalkPicarx& picarx, XWalkSelfDrive& selfDrive,
    hal::contextpointer context, const XWalkControllerCallbacks& backendCallbacks)
    : picarxObject(&picarx), selfDriveObject(&selfDrive),
      callbackContext(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
    selfDriveObject->setCancellation(callbackContext, callbacks.continueOperation);
}

/** @brief Constructs a CLI with local voice-chatbot support. */
XWalkController::XWalkController(XWalkPicarx& picarx,
    XWalkLocalVoiceChatbot& localVoiceChatbot, hal::contextpointer context,
    const XWalkControllerCallbacks& backendCallbacks)
    : picarxObject(&picarx), localVoiceChatbotObject(&localVoiceChatbot),
      callbackContext(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
}

/**
 * @brief Constructs a CLI containing only an SPI transfer Agent.
 * @param[in] spiTransfer SPI Agent that must outlive this CLI.
 * @param[in,out] context Optional platform callback context.
 * @param[in] backendCallbacks Complete non-null synchronous callback table.
 */
XWalkController::XWalkController(XWalkSpiTransfer& spiTransfer,
    hal::contextpointer context,
    const XWalkControllerCallbacks& backendCallbacks):
    spiTransferObject(&spiTransfer), callbackContext(context),
    callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
}

/**
 * @brief Constructs a CLI containing only a passive preflight report.
 * @param[in] doctorLines Report lines that must outlive this CLI.
 * @param[in,out] context Optional platform callback context.
 * @param[in] backendCallbacks Complete non-null synchronous callback table.
 */
XWalkController::XWalkController(const hal::stringvector& doctorLines,
    hal::contextpointer context,
    const XWalkControllerCallbacks& backendCallbacks):
    doctorLinesObject(&doctorLines), callbackContext(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
}

/** @brief Constructs a CLI with wake-word movement-control support. */
XWalkController::XWalkController(XWalkPicarx& picarx,
    XWalkVoiceControlledCar& voiceControlledCar, hal::contextpointer context,
    const XWalkControllerCallbacks& backendCallbacks)
    : picarxObject(&picarx), voiceControlledCarObject(&voiceControlledCar),
      callbackContext(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
}

/** @brief Constructs a CLI with the spoken movement demonstration. */
XWalkController::XWalkController(XWalkPicarx& picarx,
    XWalkVoicePromptCar& voicePromptCar, hal::contextpointer context,
    const XWalkControllerCallbacks& backendCallbacks)
    : picarxObject(&picarx), voicePromptCarObject(&voicePromptCar),
      callbackContext(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
}

/** @brief Constructs a CLI with sensor-aware voice-active-car support. */
XWalkController::XWalkController(XWalkPicarx& picarx,
    XWalkVoiceActiveCar& voiceActiveCar, hal::contextpointer context,
    const XWalkControllerCallbacks& backendCallbacks)
    : picarxObject(&picarx), voiceActiveCarObject(&voiceActiveCar),
      callbackContext(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Destroys the CLI without changing or releasing its dependencies. */
XWalkController::~XWalkController() = default;

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates that every required callback is non-null.
 * @param[in] backendCallbacks Callback table to validate.
 * @throws std::invalid_argument If any callback is null.
 */
void XWalkController::validateCallbacks(const XWalkControllerCallbacks& backendCallbacks)
{
    if ((backendCallbacks.output == nullptr) || (backendCallbacks.input == nullptr) ||
        (backendCallbacks.delay == nullptr) ||
        (backendCallbacks.continueOperation == nullptr) ||
        (backendCallbacks.sound == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("PiCar-X CLI callbacks must be complete");
    }
}

/** @brief Writes one complete output line through the injected backend. */
void XWalkController::output(hal::stringview line) const
{
    callbacks.output(callbackContext, line);
}

/** @brief Requests one response through the injected backend. */
hal::string XWalkController::input(hal::stringview prompt) const
{
    return callbacks.input(callbackContext, prompt);
}

/** @brief Delays through the injected backend. */
void XWalkController::delay(hal::uint32 durationMs) const
{
    callbacks.delay(callbackContext, durationMs);
}

} /* namespace xwalk::agent */
