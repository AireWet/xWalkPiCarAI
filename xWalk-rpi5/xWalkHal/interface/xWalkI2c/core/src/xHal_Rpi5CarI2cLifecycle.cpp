/******************************************************************************
 * @file        xHal_Rpi5CarI2cLifecycle.cpp
 * @brief       Implements lifecycle operations for the xWalk I2C interface.
 *
 * @details
 * Validates callback bindings during construction, emits xWalk lifecycle and
 * validation diagnostics, and provides the default destruction behavior for
 * the non-owning callback interface.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c
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

#include "xHal_Rpi5CarI2c.h"

#include "xHal_Rpi5CarTrace.h"

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
     * @brief Constructs an I2C interface bound to backend callbacks.
     *
     * @param[in] context
     * Non-owning opaque backend context forwarded to every callback.
     *
     * @param[in] probeOperation
     * Callback used to test whether a device responds at an address.
     *
     * @param[in] writeRegisterOperation
     * Callback used to write bytes to a device register.
     *
     * @param[in] readOperation
     * Callback used to read bytes directly from a device.
     *
     * @param[in] readRegisterOperation
     * Optional callback used to read bytes beginning at a register address.
     *
     * @param[in] tryWriteRegisterOperation
     * Optional non-throwing callback used by fail-safe register writes.
     *
     * @pre
     * The context satisfies the requirements of every configured callback and
     * remains valid for every operation performed through this object.
     *
     * @post
     * Required callback members are non-null when construction succeeds. The
     * register-read and fail-safe-write callbacks remain nullable for compatible
     * consumers.
     *
     * @throws std::invalid_argument
     * If a required probe, write-register, or sequential-read callback is null.
     */
    XWalkI2c::XWalkI2c(contextpointer context,
                       i2cprobecallback probeOperation,
                       i2cwriteregistercallback writeRegisterOperation,
                       i2creadcallback readOperation,
                       i2creadregistercallback readRegisterOperation,
                       i2ctrywriteregistercallback tryWriteRegisterOperation)
        : contextValue(context), probeCallback(probeOperation), writeRegisterCallback(writeRegisterOperation),
          readCallback(readOperation), readRegisterCallback(readRegisterOperation),
          tryWriteRegisterCallback(tryWriteRegisterOperation)
    {
        if (this->probeCallback == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "I2C probe callback must not be null");
        }
        if (this->writeRegisterCallback == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "I2C write-register callback must not be null");
        }
        if (this->readCallback == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "I2C read callback must not be null");
        }
        XWALK_HAL_TRACE_UID2(RPI .009,
                             "I2C callback interface constructed with register-read=%u and "
                             "fail-safe-write=%u",
                             static_cast<uint32>(this->readRegisterCallback != nullptr),
                             static_cast<uint32>(this->tryWriteRegisterCallback != nullptr));
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Destroys the callback interface.
     *
     * @note
     * The backend context is non-owning and is not released.
     */
    XWalkI2c::~XWalkI2c() = default;

} /* namespace xwalk::hal */
