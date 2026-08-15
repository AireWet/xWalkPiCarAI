/******************************************************************************
 * @file        xHal_Rpi5CarAdc.h
 * @brief       Declares the Robot HAT analog-to-digital converter interface.
 *
 * @details
 * Defines channel selection, raw sample acquisition, and voltage conversion for the Robot HAT ADC.
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

#ifndef XHAL_RPI5CAR_ADC_H
#define XHAL_RPI5CAR_ADC_H

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "xHal_Rpi5CarI2c.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkAdc
     * @brief Acquires analog samples from one Robot HAT ADC channel.
     *
     * @details
     * Stores a non-owning I2C dependency, maps logical channels 0 through 7 to the hardware command,
     * writes that command, reads the big-endian sample, and converts samples to volts.
     */
    class XWalkAdc
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Non-owning pointer to the I2C interface used for every transaction.
             *
             * @note
             * Never null after construction. The pointed-to object must outlive this ADC object.
             */
            XWalkI2c* i2cObject{nullptr};

            /** @brief Selected seven-bit ADC address, either 0x14 or 0x15 by automatic selection. */
            uint8 addressValue{};

            /** @brief Caller-visible logical channel in the range 0 through 7. */
            uint8 channelValue{};

            /** @brief Hardware read command containing the reversed channel mapping and read flag. */
            uint8 commandValue{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Parses a channel name in the form `A0` through `A7`.
             *
             * @param[in] channel
             * Case-sensitive ADC channel name.
             *
             * @return
             * Numeric channel in the range 0 through 7.
             *
             * @throws std::invalid_argument
             * If the name is not exactly `A0` through `A7`.
             */
            static uint8 parseChannel(stringview channel);

            /**
             * @brief Selects an explicit or automatically detected ADC address.
             *
             * @param[in] i2c
             * I2C interface used to probe candidate addresses.
             *
             * @param[in] address
             * Optional explicit seven-bit address.
             *
             * @return
             * Explicit address, first responding candidate, or 0x14 when neither candidate responds.
             */
            static uint8 selectAddress(XWalkI2c& i2c, optionaluint8 address);

            /**
             * @brief Validates and converts a logical channel to its hardware command.
             *
             * @param[in] channel
             * Logical channel in the range 0 through 7.
             *
             * @return
             * Hardware ADC read command.
             *
             * @throws std::out_of_range
             * If `channel` exceeds 7.
             */
            static uint8 createCommand(uint32 channel);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs an ADC interface for a numeric channel.
             *
             * @param[in] i2c
             * I2C dependency that must outlive this object.
             *
             * @param[in] channel
             * Logical ADC channel in the range 0 through 7.
             *
             * @param[in] address
             * Optional explicit seven-bit device address; omission probes 0x14 then 0x15.
             *
             * @throws std::out_of_range
             * If `channel` exceeds 7 or an explicit address exceeds the seven-bit I2C range.
             */
            XWalkAdc(XWalkI2c& i2c, uint32 channel, optionaluint8 address = {});

            /**
             * @brief Constructs an ADC interface for a named channel.
             *
             * @param[in] i2c
             * I2C dependency that must outlive this object.
             *
             * @param[in] channel
             * Case-sensitive channel name in the form `A0` through `A7`.
             *
             * @param[in] address
             * Optional explicit seven-bit device address; omission probes 0x14 then 0x15.
             *
             * @throws std::invalid_argument
             * If `channel` is not exactly `A0` through `A7`.
             *
             * @throws std::out_of_range
             * If an explicit address exceeds the seven-bit I2C range.
             */
            XWalkAdc(XWalkI2c& i2c, stringview channel, optionaluint8 address = {});

            /** @brief Destroys the ADC interface without releasing its non-owning I2C dependency. */
            ~XWalkAdc();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction because the object represents a fixed channel binding. */
            XWalkAdc(XWalkAdc&&) = delete;
            /** @brief Disables copying of the channel binding. */
            XWalkAdc(const XWalkAdc&) = delete;
            /** @brief Disables move assignment of the channel binding. */
            XWalkAdc& operator=(XWalkAdc&&) = delete;
            /** @brief Disables copy assignment of the channel binding. */
            XWalkAdc& operator=(const XWalkAdc&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Acquires one raw ADC sample.
             *
             * @return
             * Unsigned sample assembled from the device's most-significant and least-significant bytes.
             *
             * @throws std::runtime_error
             * If the backend does not return exactly two bytes.
             */
            uint16 read();

            /**
             * @brief Acquires one ADC sample and converts it to volts.
             *
             * @return
             * Sample scaled by the 3.3-volt reference and 4095-count full scale.
             */
            float64 readVoltage();

            /** @brief Returns the selected seven-bit I2C address. */
            uint8 address() const noexcept;

            /** @brief Returns the logical ADC channel in the range 0 through 7. */
            uint8 channel() const noexcept;

            /** @brief Returns the hardware ADC read command for the selected channel. */
            uint8 command() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_ADC_H */
