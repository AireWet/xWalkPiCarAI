/******************************************************************************
 * @file        xHal_Rpi5CarSpiLifecycle.cpp
 * @brief       Implements xWalk SPI callback binding and destruction.
 *
 * @details
 * Validates the required transfer callback and retains only non-owning backend
 * state for the lifetime of the hardware-independent SPI interface.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xHal_Rpi5CarSpi.h"

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
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Binds the interface to one caller-owned SPI backend.
 * @param[in,out] context Nullable context that outlives this object.
 * @param[in] transferOperation Non-null synchronous transfer callback.
 * @throws std::invalid_argument If `transferOperation` is null.
 */
XWalkSpi::XWalkSpi(contextpointer context,
                   spitransfercallback transferOperation)
    : contextValue(context), transferCallback(transferOperation) {
  if (transferCallback == nullptr) {
    XWALK_HAL_ERROR(XWALK_INVAL, "SPI transfer callback must not be null");
  }
  XWALK_HAL_TRACE_UID0(RPI .047, "SPI callback interface constructed");
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Releases no caller-owned backend resource. */
XWalkSpi::~XWalkSpi() = default;

} /* namespace xwalk::hal */
