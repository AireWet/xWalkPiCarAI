/******************************************************************************
 * @file        xHal_Rpi5CarAdc.cpp
 * @brief       Implements ADC sample acquisition and voltage conversion.
 *
 * @details
 * Sends channel commands over I2C, combines big-endian sample bytes, and scales
 *counts to volts.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdc
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
#include "xHal_Rpi5CarAdc.h"
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
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Acquires one raw ADC sample.
     *
     * @return
     * Unsigned sample assembled from the device's most-significant and
     * least-significant bytes.
     *
     * @throws std::runtime_error
     * If the backend does not return exactly two bytes.
     */
    uint16 XWalkAdc::read()
    {
        const bytevector commandPayload{0U, 0U};
        const bytevector sampleBytes =
            i2cObject->writeRegisterThenRead(addressValue, commandValue, commandPayload, XHAL_RPI5CAR_ADC_READ_LENGTH);
        const hal::boolean sampleBytesDifferent =
            static_cast<hal::boolean>(sampleBytes.size() != XHAL_RPI5CAR_ADC_READ_LENGTH);
        if (sampleBytesDifferent)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "ADC read did not return two bytes");
        }

        const uint16 mostSignificantByte = static_cast<uint16>(sampleBytes[0U]);
        const uint16 leastSignificantByte = static_cast<uint16>(sampleBytes[1U]);
        const uint16 shiftedMostSignificantByte = static_cast<uint16>(mostSignificantByte << 8U);
        const uint16 sample = static_cast<uint16>(shiftedMostSignificantByte | leastSignificantByte);
        XWALK_HAL_TRACE_UID2(RPI .175, "ADC channel %u read sample %u", channelValue, sample);
        return sample;
    }

    /**
     * @brief Acquires one ADC sample and converts it to volts.
     *
     * @return
     * Sample scaled by the 3.3-volt reference and 4095-count full scale.
     */
    float64 XWalkAdc::readVoltage()
    {
        const uint16 adcCount = read();
        const float64 adcCountValue = static_cast<float64>(adcCount);
        const float64 referenceVoltage = static_cast<float64>(XHAL_RPI5CAR_ADC_REFERENCE_VOLTAGE);
        const float64 maximumAdcCount = static_cast<float64>(XHAL_RPI5CAR_ADC_MAX_COUNT);
        const float64 scaledVoltage = adcCountValue * referenceVoltage;
        const float64 voltage = scaledVoltage / maximumAdcCount;
        XWALK_HAL_TRACE_UID2(RPI .176, "ADC channel %u converted sample to %.6f volts", channelValue, voltage);
        return voltage;
    }

    /**
     * @brief Returns the selected seven-bit I2C address.
     *
     * @return
     * Selected address used for all ADC transactions.
     */
    uint8 XWalkAdc::address() const noexcept
    {
        return addressValue;
    }

    /**
     * @brief Returns the logical ADC channel.
     *
     * @return
     * Channel in the range 0 through 7.
     */
    uint8 XWalkAdc::channel() const noexcept
    {
        return channelValue;
    }

    /**
     * @brief Returns the hardware ADC read command.
     *
     * @return
     * Reversed channel index combined with the ADC read-command bit.
     */
    uint8 XWalkAdc::command() const noexcept
    {
        return commandValue;
    }

} /* namespace xwalk::hal */
