/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestI2c.h
 * @brief       Declares the in-memory I2C test double used by PWM host tests.
 *
 * @details
 * Records probes, destination addresses, registers, and payloads while exposing
 * callbacks for an externally created `XWalkI2c` interface.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Test
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

#ifndef XHAL_RPI5CAR_PWM_TEST_I2C_H
#define XHAL_RPI5CAR_PWM_TEST_I2C_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2c.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkPwmTestI2c
     * @brief Simulates I2C discovery and records PWM register traffic.
     *
     * @details
     * Exposes public static callbacks so each test function can create its own
     * `XWalkI2c` object and bind that object's non-owning context to this test
     * double. Recorded vectors remain valid for the lifetime of this object.
     */
    class XWalkPwmTestI2c
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Set of seven-bit addresses configured to answer probes. */
            byteset presentAddresses;
            /** @brief Ordered record of all probed seven-bit addresses. */
            bytevector probedAddresses;
            /** @brief Ordered record of all register-write destination addresses. */
            bytevector writeAddresses;
            /** @brief Ordered record of all written eight-bit register addresses. */
            bytevector writeRegisters;
            /** @brief Ordered deep copies of all register-write payloads. */
            bytevectorvector writePayloads;

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /** @brief Constructs an empty in-memory I2C recording backend. */
            XWalkPwmTestI2c();

            /** @brief Destroys the test double and its recorded data. */
            ~XWalkPwmTestI2c();

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Records a probe and reports configured address presence.
             *
             * @param[in,out] context
             * Non-null pointer to the `XWalkPwmTestI2c` object created before the
             * separate `XWalkI2c` callback interface.
             *
             * @param[in] address
             * Seven-bit I2C address to record.
             *
             * @return
             * `true` when the address is present in `presentAddresses`.
             */
            static boolean probeCallback(contextpointer context, uint8 address);

            /**
             * @brief Records one simulated register write.
             *
             * @param[in,out] context
             * Non-null pointer to the `XWalkPwmTestI2c` object created before the
             * separate `XWalkI2c` callback interface.
             *
             * @param[in] address
             * Seven-bit destination address.
             *
             * @param[in] reg
             * Eight-bit destination register address.
             *
             * @param[in] data
             * Payload bytes copied into test storage.
             */
            static void writeRegisterCallback(contextpointer context, uint8 address, uint8 reg, const bytevector& data);

            /**
             * @brief Returns zero-filled bytes for interfaces that require a read callback.
             *
             * @param[in] context
             * Non-owning callback context; unused by this implementation.
             *
             * @param[in] address
             * Seven-bit source address; unused by this implementation.
             *
             * @param[in] length
             * Number of zero-filled bytes to return.
             *
             * @return
             * A byte vector of size `length` containing zero values.
             */
            static bytevector readCallback(contextpointer context, uint8 address, size length);

            /**
             * @brief Returns the number of recorded register writes.
             *
             * @return
             * Number of writes recorded since the last clear.
             */
            size writeCount() const noexcept;

            /** @brief Clears write records without changing probes or present addresses. */
            void clearWrites() noexcept;

            /**
             * @brief Configures an address to answer subsequent probes.
             *
             * @param[in] address
             * Seven-bit I2C address to mark present.
             */
            void addPresentAddress(uint8 address);

            /**
             * @brief Returns a recorded destination address.
             *
             * @param[in] index
             * Zero-based write-record index.
             *
             * @return
             * Destination address stored at `index`.
             */
            uint8 writeAddress(size index) const;

            /**
             * @brief Returns a recorded register address.
             *
             * @param[in] index
             * Zero-based write-record index.
             *
             * @return
             * Register address stored at `index`.
             */
            uint8 writeRegister(size index) const;

            /**
             * @brief Returns the complete ordered probe record.
             *
             * @return
             * Read-only reference to the probe-address sequence.
             */
            const bytevector& probes() const noexcept;

            /**
             * @brief Returns a recorded register payload.
             *
             * @param[in] index
             * Zero-based write-record index.
             *
             * @return
             * Read-only reference to the payload stored at `index`.
             */
            const bytevector& writeData(size index) const;
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_PWM_TEST_I2C_H */
