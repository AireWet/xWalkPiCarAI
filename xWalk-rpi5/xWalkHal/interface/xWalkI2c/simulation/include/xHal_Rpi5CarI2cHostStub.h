/******************************************************************************
 * @file        xHal_Rpi5CarI2cHostStub.h
 * @brief       Declares the device-free xWalkI2c host stub.
 *
 * @details
 * Mirrors Linux device and SMBus requests into owned state and returns
 * deterministic results without opening physical I2C.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-09
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_I2C_HOST_STUB_H
#define XHAL_RPI5CAR_I2C_HOST_STUB_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2cDevice.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free simulation components for the xWalk HAL.
 */
namespace xwalk::hal::sim
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkI2cHostStub
 * @brief Mirrors I2C callback traffic for device-free host execution.
 *
 * @details
 * Owns the latest mirrored address, register, payload, and read length. It
 * implements the device-operation boundary injected into `XWalkI2cLinux`, so
 * the host runner executes the same backend logic as the Raspberry Pi path.
 */
class XWalkI2cHostStub final: public XWalkI2cDevice
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Most recently mirrored seven-bit I2C address. */
        uint8 lastAddressValue{};

        /** @brief Most recently mirrored eight-bit register address. */
        uint8 lastRegisterValue{};

        /** @brief Owned copy of the most recently mirrored write payload. */
        bytevector lastDataValue;

        /** @brief Most recently mirrored read length in bytes. */
        size lastReadLengthValue{};

        /** @brief Current byte index for a mirrored sequential read. */
        size sequentialReadIndex{};

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /** @brief Constructs an empty device-free I2C mirror. */
        XWalkI2cHostStub();

        /** @brief Destroys the mirror and its owned callback records. */
        ~XWalkI2cHostStub();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        XWalkI2cHostStub(XWalkI2cHostStub&&) = delete;
        XWalkI2cHostStub(const XWalkI2cHostStub&) = delete;
        XWalkI2cHostStub& operator=(XWalkI2cHostStub&&) = delete;
        XWalkI2cHostStub& operator=(const XWalkI2cHostStub&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /** @brief Opens a logical host-mirror device descriptor. */
        int32 openDevice(cstring devicePath) override;

        /** @brief Mirrors Linux slave-address selection. */
        boolean selectAddress(int32 fileDescriptor, uint8 address) override;

        /** @brief Mirrors one Linux SMBus transfer request. */
        boolean transfer(int32 fileDescriptor, contextpointer request) override;

        /** @brief Mirrors descriptor closure without accessing hardware. */
        void closeDevice(int32 fileDescriptor) noexcept override;

        /**
         * @brief Returns the most recently mirrored I2C address.
         * @return Seven-bit address from the latest callback request.
         */
        uint8 lastAddress() const noexcept;

        /**
         * @brief Returns the most recently mirrored register address.
         * @return Eight-bit register from the latest register callback request.
         */
        uint8 lastRegister() const noexcept;

        /**
         * @brief Returns the most recently mirrored write payload.
         * @return Read-only reference valid for this stub's lifetime or until the next write.
         */
        const bytevector& lastData() const noexcept;

        /**
         * @brief Returns the most recently mirrored read length in bytes.
         * @return Requested sequential-read or register-read length in bytes.
         */
        size lastReadLength() const noexcept;
};

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_I2C_HOST_STUB_H */
