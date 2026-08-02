/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345.cpp
 * @brief       Implements ADXL345 configuration and acceleration acquisition.
 *
 * @details
 * Configures measurement mode, performs the Python-compatible discarded read,
 * converts little-endian signed samples, and scales counts to standard gravity.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdxl345
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

#include "xHal_Rpi5CarAdxl345.h"
#include "xHal_Rpi5CarExceptions.h"

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
 * @brief Writes the data-format and measurement-mode configuration.
 *
 * @post
 * The device is configured with format value zero and measurement bit eight.
 */
void XWalkAdxl345::configureMeasurement()
{
    const bytevector formatData{XHAL_RPI5CAR_ADXL345_DATA_FORMAT_VALUE};
    i2cObject->writeRegister(addressValue, XHAL_RPI5CAR_ADXL345_DATA_FORMAT_REGISTER, formatData);
    const bytevector powerData{XHAL_RPI5CAR_ADXL345_MEASUREMENT_MODE_VALUE};
    i2cObject->writeRegister(addressValue, XHAL_RPI5CAR_ADXL345_POWER_CONTROL_REGISTER, powerData);
}

/**
 * @brief Converts one little-endian sample to standard gravity.
 *
 * @param[in] sampleBytes
 * Exactly two bytes ordered low byte then high byte.
 *
 * @return
 * Signed acceleration in units of standard gravity.
 *
 * @throws std::runtime_error
 * If `sampleBytes` does not contain exactly two bytes.
 */
float64 XWalkAdxl345::convertSample(const bytevector& sampleBytes)
{
    if (sampleBytes.size() != XHAL_RPI5CAR_ADXL345_SAMPLE_LENGTH)
    {
        XHAL_THROW_RUNTIME_ERROR("ADXL345 read did not return two bytes");
    }

    const uint16 lowByteValue = static_cast<uint16>(sampleBytes[0U]);
    const uint16 highByteValue = static_cast<uint16>(sampleBytes[1U]);
    const uint16 shiftedHighByte = static_cast<uint16>(highByteValue << 8U);
    const uint16 packedSample = static_cast<uint16>(lowByteValue | shiftedHighByte);
    int32 signedCount = static_cast<int32>(packedSample);
    if ((packedSample & XHAL_RPI5CAR_ADXL345_SIGN_BIT) != 0U)
    {
        signedCount -= XHAL_RPI5CAR_ADXL345_SIGNED_MODULUS;
    }

    const float64 signedCountValue = static_cast<float64>(signedCount);
    const float64 countsPerGravity = static_cast<float64>(XHAL_RPI5CAR_ADXL345_COUNTS_PER_G);
    return signedCountValue / countsPerGravity;
}

/**
 * @brief Reads and converts one validated axis.
 *
 * @param[in] axis
 * Validated zero-based axis index.
 *
 * @return
 * Signed acceleration in units of standard gravity.
 *
 * @throws std::runtime_error
 * If either register read does not return exactly two bytes.
 */
float64 XWalkAdxl345::readAxis(uint8 axis)
{
    configureMeasurement();
    const uint8 dataRegister = registerForAxis(axis);
    const bytevector discardedSample = i2cObject->readRegister(addressValue, dataRegister,
        XHAL_RPI5CAR_ADXL345_SAMPLE_LENGTH);
    static_cast<void>(convertSample(discardedSample));
    const bytevector sampleBytes = i2cObject->readRegister(addressValue, dataRegister,
        XHAL_RPI5CAR_ADXL345_SAMPLE_LENGTH);
    return convertSample(sampleBytes);
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Reads acceleration from one axis.
 *
 * @param[in] axis
 * Axis to acquire.
 *
 * @return
 * Signed acceleration in units of standard gravity.
 *
 * @throws std::out_of_range
 * If `axis` does not identify X, Y, or Z.
 *
 * @throws std::runtime_error
 * If register reads are unavailable or do not return exactly two bytes.
 */
float64 XWalkAdxl345::read(XWalkAdxl345Axis axis)
{
    return readAxis(validateAxis(axis));
}

/**
 * @brief Reads acceleration from all three axes.
 *
 * @return
 * X-, Y-, and Z-axis acceleration in units of standard gravity.
 *
 * @throws std::runtime_error
 * If register reads are unavailable or do not return exactly two bytes.
 */
adxl345values XWalkAdxl345::read()
{
    adxl345values values{};
    values[XHAL_RPI5CAR_ADXL345_X_AXIS] = readAxis(XHAL_RPI5CAR_ADXL345_X_AXIS);
    values[XHAL_RPI5CAR_ADXL345_Y_AXIS] = readAxis(XHAL_RPI5CAR_ADXL345_Y_AXIS);
    values[XHAL_RPI5CAR_ADXL345_Z_AXIS] = readAxis(XHAL_RPI5CAR_ADXL345_Z_AXIS);
    return values;
}

/**
 * @brief Returns the configured seven-bit I2C address.
 *
 * @return
 * Address used for all ADXL345 transactions.
 */
uint8 XWalkAdxl345::address() const noexcept
{
    return addressValue;
}

} /* namespace xwalk::hal */
