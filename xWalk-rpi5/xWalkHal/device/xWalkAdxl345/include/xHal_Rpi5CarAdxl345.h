/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345.h
 * @brief       Declares the ADXL345 three-axis accelerometer interface.
 *
 * @details
 * Defines measurement configuration, register-addressed sample acquisition,
 * signed conversion, and acceleration scaling for a caller-owned I2C object.
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

#ifndef XHAL_RPI5CAR_ADXL345_H
#define XHAL_RPI5CAR_ADXL345_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAdxl345Types.h"
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
     * @class XWalkAdxl345
     * @brief Acquires acceleration from a caller-owned ADXL345 I2C device.
     *
     * @details
     * Configures the sensor for measurement, discards the first sample from each
     * axis as required by the Python implementation, and returns signed values in
     * units of standard gravity.
     */
    class XWalkAdxl345
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Non-owning pointer to the I2C interface used for every transaction.
             *
             * @note
             * Never null after construction and must outlive this accelerometer.
             */
            XWalkI2c* i2cObject{nullptr};

            /** @brief Validated seven-bit ADXL345 I2C address. */
            uint8 addressValue{XHAL_RPI5CAR_ADXL345_ADDRESS};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Validates an axis selector.
             *
             * @param[in] axis
             * Axis expected to be X, Y, or Z.
             *
             * @return
             * Validated zero-based axis index.
             *
             * @throws std::out_of_range
             * If `axis` does not identify X, Y, or Z.
             */
            static uint8 validateAxis(XWalkAdxl345Axis axis);

            /**
             * @brief Returns the first data register for an axis.
             *
             * @param[in] axis
             * Validated zero-based axis index.
             *
             * @return
             * Low-byte data register for the selected axis.
             */
            static uint8 registerForAxis(uint8 axis) noexcept;

            /**
             * @brief Writes the data-format and measurement-mode configuration.
             *
             * @post
             * The device is configured with format value zero and measurement bit eight.
             */
            void configureMeasurement();

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
            float64 readAxis(uint8 axis);

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
            static float64 convertSample(const bytevector& sampleBytes);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs an ADXL345 interface.
             *
             * @param[in] i2c
             * I2C dependency that must outlive this object and provide register reads.
             *
             * @param[in] address
             * Seven-bit ADXL345 address; defaults to `0x53`.
             *
             * @throws std::out_of_range
             * If `address` exceeds the seven-bit I2C range.
             */
            explicit XWalkAdxl345(XWalkI2c& i2c, uint8 address = XHAL_RPI5CAR_ADXL345_ADDRESS);

            /** @brief Destroys the interface without releasing its non-owning I2C dependency. */
            ~XWalkAdxl345();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction to preserve dependency identity. */
            XWalkAdxl345(XWalkAdxl345&&) = delete;
            /** @brief Disables copying of the I2C dependency binding. */
            XWalkAdxl345(const XWalkAdxl345&) = delete;
            /** @brief Disables move assignment of the I2C dependency binding. */
            XWalkAdxl345& operator=(XWalkAdxl345&&) = delete;
            /** @brief Disables copy assignment of the I2C dependency binding. */
            XWalkAdxl345& operator=(const XWalkAdxl345&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

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
            float64 read(XWalkAdxl345Axis axis);

            /**
             * @brief Reads acceleration from all three axes.
             *
             * @return
             * X-, Y-, and Z-axis acceleration in units of standard gravity.
             *
             * @throws std::runtime_error
             * If register reads are unavailable or do not return exactly two bytes.
             */
            adxl345values read();

            /**
             * @brief Returns the configured seven-bit I2C address.
             *
             * @return
             * Address used for all ADXL345 transactions.
             */
            uint8 address() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_ADXL345_H */
