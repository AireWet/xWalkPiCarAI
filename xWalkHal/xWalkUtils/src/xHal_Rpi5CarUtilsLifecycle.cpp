/******************************************************************************
 * @file        xHal_Rpi5CarUtilsLifecycle.cpp
 * @brief       Defines xWalk utility construction and validation.
 *
 * @details
 * Validates and retains caller-owned platform-service bindings without
 * acquiring or releasing terminal, process, network, or operating-system state.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils
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

#include "xHal_Rpi5CarUtils.h"

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
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a utility coordinator from caller-owned services.
 *
 * @param[in,out] backendContext
 * Nullable non-owning context used by every platform callback.
 *
 * @param[in] backendCallbacks
 * Complete callback table copied by value.
 */
XWalkUtils::XWalkUtils(contextpointer backendContext,
    const XWalkUtilsCallbacks& backendCallbacks)
    : backendContextPointer(backendContext), callbacks(backendCallbacks)
{
    validateCallbacks(backendCallbacks);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Destroys the coordinator without releasing platform resources. */
XWalkUtils::~XWalkUtils() = default;

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates that every required platform callback is non-null.
 *
 * @param[in] backendCallbacks
 * Callback table inspected before any operation is possible.
 */
void XWalkUtils::validateCallbacks(const XWalkUtilsCallbacks& backendCallbacks)
{
    if ((backendCallbacks.output == nullptr) ||
        (backendCallbacks.setVolume == nullptr) ||
        (backendCallbacks.runCommand == nullptr) ||
        (backendCallbacks.executableExists == nullptr) ||
        (backendCallbacks.ipAddress == nullptr) ||
        (backendCallbacks.username == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Utility callbacks must not be null");
    }
}

/**
 * @brief Validates a terminal-color enumerator before dispatch.
 *
 * @param[in] color
 * Color value to validate.
 *
 * @return
 * Validated color value.
 */
XWalkUtilityColor XWalkUtils::validateColor(XWalkUtilityColor color)
{
    switch (color)
    {
        case XWalkUtilityColor::Gray:
        case XWalkUtilityColor::Red:
        case XWalkUtilityColor::Green:
        case XWalkUtilityColor::Yellow:
        case XWalkUtilityColor::Blue:
        case XWalkUtilityColor::Purple:
        case XWalkUtilityColor::DarkGreen:
        case XWalkUtilityColor::White:
            return color;
        default:
            XHAL_THROW_OUT_OF_RANGE("Utility color is not supported");
    }
}

} /* namespace xwalk::hal */
