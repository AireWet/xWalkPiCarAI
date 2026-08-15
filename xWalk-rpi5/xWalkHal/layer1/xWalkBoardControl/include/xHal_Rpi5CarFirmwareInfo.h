/******************************************************************************
 * @file        xHal_Rpi5CarFirmwareInfo.h
 * @brief       Declares Robot HAT firmware-version acquisition.
 *
 * @details
 * Selects a responding Robot HAT I2C address and exposes typed and formatted
 * firmware versions together with the ported Python library version.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl
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

#ifndef XHAL_RPI5CAR_FIRMWARE_INFO_H
#define XHAL_RPI5CAR_FIRMWARE_INFO_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarFirmwareInfoTypes.h"
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
     * @class XWalkFirmwareInfo
     * @brief Reads Robot HAT firmware-version information over injected I2C.
     *
     * @details
     * Probes supported addresses in Python-compatible order, stores the selected
     * address, and performs atomic register reads through a caller-owned interface.
     */
    class XWalkFirmwareInfo
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Non-owning pointer to the I2C interface used for every operation.
             *
             * @note
             * Never null and must outlive this firmware-information object.
             */
            XWalkI2c* i2cPointer{nullptr};

            /** @brief First responding supported seven-bit Robot HAT I2C address. */
            uint8 addressValue{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Selects the first responding supported Robot HAT address.
             *
             * @param[in,out] i2c
             * Caller-owned interface used to probe addresses `0x14` then `0x15`.
             *
             * @return
             * First responding supported seven-bit address.
             *
             * @throws std::runtime_error
             * If neither supported address responds.
             */
            static uint8 selectAddress(XWalkI2c& i2c);

            /**
             * @brief Formats a typed firmware version as dotted decimal text.
             *
             * @param[in] version
             * Major, minor, and patch components to format.
             *
             * @return
             * Owned text in `major.minor.patch` form.
             */
            static string formatVersion(const XWalkFirmwareVersion& version);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a reader and selects one responding Robot HAT address.
             *
             * @param[in,out] i2c
             * I2C dependency that must outlive this object and support register reads.
             *
             * @post
             * `address()` identifies the first responding supported address.
             *
             * @throws std::runtime_error
             * If neither supported Robot HAT address responds.
             */
            explicit XWalkFirmwareInfo(XWalkI2c& i2c);

            /** @brief Destroys the reader without releasing its non-owning I2C dependency. */
            ~XWalkFirmwareInfo();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction to preserve dependency identity. */
            XWalkFirmwareInfo(XWalkFirmwareInfo&&) = delete;
            /** @brief Disables copying of the I2C dependency binding. */
            XWalkFirmwareInfo(const XWalkFirmwareInfo&) = delete;
            /** @brief Disables move assignment to preserve dependency identity. */
            XWalkFirmwareInfo& operator=(XWalkFirmwareInfo&&) = delete;
            /** @brief Disables copy assignment of the I2C dependency binding. */
            XWalkFirmwareInfo& operator=(const XWalkFirmwareInfo&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Reads the current Robot HAT firmware version.
             *
             * @return
             * Major, minor, and patch values from three consecutive register bytes.
             *
             * @throws std::runtime_error
             * If the backend lacks atomic register reads or returns another byte count.
             */
            XWalkFirmwareVersion read();

            /**
             * @brief Reads and formats the current Robot HAT firmware version.
             *
             * @return
             * Owned dotted-decimal text in `major.minor.patch` form.
             *
             * @throws std::runtime_error
             * If firmware acquisition fails.
             */
            string readText();

            /**
             * @brief Returns the selected Robot HAT I2C address.
             *
             * @return
             * First responding supported address selected during construction.
             */
            uint8 address() const noexcept;

            /**
             * @brief Returns the ported Robot HAT Python library version.
             *
             * @return
             * Static non-owning text view containing `2.5.5`.
             */
            static stringview libraryVersion() noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_FIRMWARE_INFO_H */
