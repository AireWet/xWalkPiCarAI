/******************************************************************************
 * @file        xHal_Rpi5CarI2cLinux.cpp
 * @brief       Implements Linux I2C probe, register-write, and read operations.
 *
 * @details
 * Uses Linux `i2c-dev` ioctl requests for serialized address selection, device
 * probing, SMBus byte or word writes, and I2C block writes with bounded retries.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Linux Backend
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

#include "xHal_Rpi5CarI2cLinux.h"

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
 * Private address-selection member function definitions
 ******************************************************************************/

/**
 * @brief Selects the Linux I2C slave address for the next transaction.
 *
 * @param[in] address
 * Seven-bit I2C address passed to the Linux driver.
 *
 * @return
 * `true` when the `I2C_SLAVE` ioctl succeeds; otherwise `false`.
 *
 * @pre
 * `fileDescriptor` identifies an open Linux I2C device.
 */
boolean XWalkI2cLinux::selectAddress(uint8 address)
{
    return ::ioctl(fileDescriptor, I2C_SLAVE, address) >= 0;
}

/******************************************************************************
 * Public probe member function definitions
 ******************************************************************************/

/**
 * @brief Probes an I2C address using bounded retries.
 *
 * @details
 * Holds the bus mutex while selecting the address and issuing SMBus quick-write
 * requests so another execution context cannot interleave a transaction.
 *
 * @param[in] address
 * Seven-bit I2C address to probe.
 *
 * @return
 * `true` if any attempt receives a response; otherwise `false`.
 *
 * @pre
 * The backend owns a valid open Linux I2C descriptor.
 *
 * @throws std::out_of_range
 * If `address` exceeds the seven-bit protocol range.
 */
boolean XWalkI2cLinux::probeDevice(uint8 address)
{
    common::validateI2cAddress(address);
    const mutexlock lock(mutex);
    for (uint32 attempt = 0U; attempt < retryCountValue; ++attempt)
    {
        const hal::boolean addressSelected = selectAddress(address);
        if (addressSelected == false)
        {
            continue;
        }

        i2c_smbus_ioctl_data request{};
        request.read_write = I2C_SMBUS_WRITE;
        request.command = 0U;
        request.size = I2C_SMBUS_QUICK;
        request.data = nullptr;

        const hal::boolean transferSucceeded =
            static_cast<hal::boolean>(
                ::ioctl(fileDescriptor, I2C_SMBUS, &request) >= 0);
        if (transferSucceeded)
        {
            return true;
        }
    }
    return false;
}

/******************************************************************************
 * Private read member function definitions
 ******************************************************************************/

/**
 * @brief Reads one byte from the currently selected Linux I2C device.
 *
 * @param[out] value
 * Byte returned by the Linux SMBus request when the operation succeeds.
 *
 * @return
 * `true` when the Linux ioctl succeeds; otherwise `false`.
 *
 * @pre
 * The caller holds the bus mutex and has selected the destination address.
 */
boolean XWalkI2cLinux::readByteOnce(uint8& value)
{
    i2c_smbus_data smbusData{};
    i2c_smbus_ioctl_data request{};
    request.read_write = I2C_SMBUS_READ;
    request.command = 0U;
    request.size = I2C_SMBUS_BYTE;
    request.data = &smbusData;

    const boolean readSucceeded = ::ioctl(fileDescriptor, I2C_SMBUS, &request) >= 0;
    if (readSucceeded)
    {
        value = smbusData.byte;
    }
    return readSucceeded;
}

/******************************************************************************
 * Public read member function definitions
 ******************************************************************************/

/**
 * @brief Reads bytes from an I2C device using a serialized retry loop.
 *
 * @param[in] address
 * Seven-bit I2C source address.
 *
 * @param[in] length
 * Number of bytes requested in the range 1 to 32.
 *
 * @return
 * Exactly `length` bytes in bus order when the transaction succeeds.
 *
 * @throws std::invalid_argument
 * If `length` is zero.
 *
 * @throws std::out_of_range
 * If the address or length exceeds its supported range.
 *
 * @throws std::runtime_error
 * If every configured read attempt fails.
 */
bytevector XWalkI2cLinux::readDevice(uint8 address, size length)
{
    common::validateI2cAddress(address);
    if (length == 0U)
    {
        XHAL_THROW_INVALID_ARGUMENT("I2C read length must not be zero");
    }
    if (length > XHAL_RPI5CAR_I2C_SMBUS_BLOCK_MAX)
    {
        XHAL_THROW_OUT_OF_RANGE("I2C read length exceeds SMBus block size");
    }

    const mutexlock lock(mutex);
    for (uint32 attempt = 0U; attempt < retryCountValue; ++attempt)
    {
        bytevector bytes;
        bytes.reserve(length);
        boolean readSucceeded = selectAddress(address);
        for (size index = 0U; (index < length) && readSucceeded; ++index)
        {
            uint8 value{};
            readSucceeded = readByteOnce(value);
            if (readSucceeded)
            {
                bytes.push_back(value);
            }
        }
        if (readSucceeded)
        {
            return bytes;
        }
    }
    XHAL_THROW_RUNTIME_ERROR("Linux I2C read failed after retries");
}

/**
 * @brief Reads consecutive bytes beginning at an I2C register.
 *
 * @details
 * Uses one SMBus I2C-block transaction while holding the bus mutex so address
 * selection, register selection, and data acquisition cannot be interleaved.
 *
 * @param[in] address
 * Seven-bit I2C source address.
 *
 * @param[in] reg
 * First eight-bit register address to read.
 *
 * @param[in] length
 * Number of bytes requested in the range 1 to 32.
 *
 * @return
 * Exactly `length` bytes in ascending register order.
 *
 * @throws std::invalid_argument
 * If `length` is zero.
 *
 * @throws std::out_of_range
 * If the address or length exceeds its supported range.
 *
 * @throws std::runtime_error
 * If every register-read attempt fails or returns an unexpected byte count.
 */
bytevector XWalkI2cLinux::readRegisterDevice(uint8 address, uint8 reg, size length)
{
    common::validateI2cAddress(address);
    if (length == 0U)
    {
        XHAL_THROW_INVALID_ARGUMENT("I2C register-read length must not be zero");
    }
    if (length > XHAL_RPI5CAR_I2C_SMBUS_BLOCK_MAX)
    {
        XHAL_THROW_OUT_OF_RANGE("I2C register-read length exceeds SMBus block size");
    }

    const mutexlock lock(mutex);
    for (uint32 attempt = 0U; attempt < retryCountValue; ++attempt)
    {
        const hal::boolean addressSelected = selectAddress(address);
        if (addressSelected == false)
        {
            continue;
        }

        i2c_smbus_data smbusData{};
        smbusData.block[0U] = static_cast<uint8>(length);
        i2c_smbus_ioctl_data request{};
        request.read_write = I2C_SMBUS_READ;
        request.command = reg;
        request.size = I2C_SMBUS_I2C_BLOCK_DATA;
        request.data = &smbusData;

        const boolean readSucceeded = ::ioctl(fileDescriptor, I2C_SMBUS, &request) >= 0;
        const size returnedLength = static_cast<size>(smbusData.block[0U]);
        if (readSucceeded && (returnedLength == length))
        {
            bytevector bytes;
            bytes.reserve(length);
            for (size index = 0U; index < length; ++index)
            {
                bytes.push_back(smbusData.block[index + 1U]);
            }
            return bytes;
        }
    }
    XHAL_THROW_RUNTIME_ERROR("Linux I2C register read failed after retries");
}

/******************************************************************************
 * Private register-write member function definitions
 ******************************************************************************/

/**
 * @brief Encodes and submits one SMBus register-write attempt.
 *
 * @details
 * Selects byte-data, word-data, or I2C block-data format from the payload size.
 * Two-byte payloads are packed according to the Linux SMBus word layout.
 *
 * @param[in] reg
 * Eight-bit destination register address.
 *
 * @param[in] payload
 * Payload containing between 1 and 32 bytes.
 *
 * @return
 * `true` when the Linux SMBus ioctl succeeds; otherwise `false`.
 *
 * @pre
 * The caller holds the bus mutex, has selected the destination address, and has
 * validated the payload length.
 */
boolean XWalkI2cLinux::writeRegisterOnce(uint8 reg, const bytevector& payload)
{
    i2c_smbus_data smbusData{};
    i2c_smbus_ioctl_data request{};
    request.read_write = I2C_SMBUS_WRITE;
    request.command = reg;
    request.data = &smbusData;

    const hal::boolean payloadMatched =
        static_cast<hal::boolean>(
            payload.size() == 1U);
    if (payloadMatched)
    {
        smbusData.byte = payload[0U];
        request.size = I2C_SMBUS_BYTE_DATA;
    }
    else {
        const hal::boolean twoBytePayload =
            static_cast<hal::boolean>(
                payload.size() == 2U);
            if (twoBytePayload)
    {
        const uint16 lowByteValue = static_cast<uint16>(payload[0U]);
        const uint16 highByteValue = static_cast<uint16>(payload[1U]);
        const uint16 shiftedHighByte = static_cast<uint16>(highByteValue << 8U);
        smbusData.word = static_cast<uint16>(lowByteValue | shiftedHighByte);

        request.size = I2C_SMBUS_WORD_DATA;
    }
    else
    {
        smbusData.block[0U] = static_cast<uint8>(payload.size());
        for (size index = 0U; index < payload.size(); ++index)
        {
            smbusData.block[index + 1U] = payload[index];
        }
        request.size = I2C_SMBUS_I2C_BLOCK_DATA;
    }
    }

    return ::ioctl(fileDescriptor, I2C_SMBUS, &request) >= 0;
}

/******************************************************************************
 * Public register-write member function definitions
 ******************************************************************************/

/**
 * @brief Writes an I2C register payload using a serialized retry loop.
 *
 * @param[in] address
 * Seven-bit I2C destination address.
 *
 * @param[in] reg
 * Eight-bit destination register address.
 *
 * @param[in] payload
 * Payload containing between 1 and 32 bytes.
 *
 * @post
 * A successful return means one complete address-selection and write sequence
 * succeeded while the bus mutex was held.
 *
 * @throws std::invalid_argument
 * If the payload is empty.
 *
 * @throws std::out_of_range
 * If `address` exceeds the seven-bit protocol range or the payload exceeds the
 * 32-byte SMBus block limit.
 *
 * @throws std::runtime_error
 * If every configured write attempt fails.
 */
void XWalkI2cLinux::writeRegisterDevice(uint8 address, uint8 reg, const bytevector& payload)
{
    common::validateI2cAddress(address);
    const hal::boolean payloadEmpty =
        static_cast<hal::boolean>(
            payload.empty());
    if (payloadEmpty)
    {
        XHAL_THROW_INVALID_ARGUMENT("I2C register data must not be empty");
    }

    const hal::boolean payloadTooLarge =
        static_cast<hal::boolean>(
            payload.size() > XHAL_RPI5CAR_I2C_SMBUS_BLOCK_MAX);
    if (payloadTooLarge)
    {
        XHAL_THROW_OUT_OF_RANGE("I2C register data exceeds SMBus block size");
    }

    const mutexlock lock(mutex);
    for (uint32 attempt = 0U; attempt < retryCountValue; ++attempt)
    {
        const hal::boolean registerWriteSucceeded =
            static_cast<hal::boolean>(
                selectAddress(address) && writeRegisterOnce(reg, payload));
        if (registerWriteSucceeded)
        {
            return;
        }
    }

    XHAL_THROW_RUNTIME_ERROR("Linux I2C register write failed after retries");
}

/**
 * @brief Attempts a validated I2C register write without throwing.
 * @return `true` when one complete address-selection and write sequence succeeds; otherwise `false`.
 */
boolean XWalkI2cLinux::tryWriteRegisterDevice(uint8 address, uint8 reg,
    const bytevector& payload) noexcept
{
    const hal::boolean addressPayloadInvalid =
        static_cast<hal::boolean>(
            (address > common::I2C_MAXIMUM_SEVEN_BIT_ADDRESS) || payload.empty() ||
        (payload.size() > XHAL_RPI5CAR_I2C_SMBUS_BLOCK_MAX));
    if (addressPayloadInvalid)
    {
        return false;
    }
    const mutexlock lock(mutex);
    for (uint32 attempt = 0U; attempt < retryCountValue; ++attempt)
    {
        const hal::boolean registerWriteSucceeded =
            static_cast<hal::boolean>(
                selectAddress(address) && writeRegisterOnce(reg, payload));
        if (registerWriteSucceeded)
        {
            return true;
        }
    }
    return false;
}

} /* namespace xwalk::hal */
