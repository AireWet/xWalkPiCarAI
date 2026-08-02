/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelLifecycle.cpp
 * @brief       Implements language-model validation and lifecycle behavior.
 *
 * @details
 * Validates a complete application backend and retains its non-owning context
 * without assuming ownership of provider or conversation resources.
 *
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel
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

#include "xHal_Rpi5CarLanguageModel.h"

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
 * @brief Validates that every required backend callback is non-null.
 *
 * @param[in] backendCallbacks
 * Callback table to validate before storing or invoking it.
 *
 * @throws std::invalid_argument
 * If any callback is null.
 */
void XWalkLanguageModel::validateCallbacks(const XWalkLanguageModelCallbacks& backendCallbacks)
{
    if ((backendCallbacks.setInstructions == nullptr) || (backendCallbacks.setWelcome == nullptr) ||
        (backendCallbacks.setMaximumMessages == nullptr) || (backendCallbacks.addMessage == nullptr) ||
        (backendCallbacks.prompt == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Language-model backend requires every callback");
    }
}

/**
 * @brief Validates a retained conversation-message limit.
 *
 * @param[in] maximumMessages
 * Requested number of retained messages.
 *
 * @throws std::out_of_range
 * If `maximumMessages` is zero.
 */
void XWalkLanguageModel::validateMaximumMessages(uint32 maximumMessages)
{
    if (maximumMessages == 0U)
    {
        XHAL_THROW_OUT_OF_RANGE("Language-model message limit must be non-zero");
    }
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a language-model coordinator from a complete backend table.
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
XWalkLanguageModel::XWalkLanguageModel(contextpointer context,
    const XWalkLanguageModelCallbacks& backendCallbacks):
    backendContextPointer(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Destroys the coordinator without releasing caller-owned backend resources. */
XWalkLanguageModel::~XWalkLanguageModel() = default;

} /* namespace xwalk::hal */
