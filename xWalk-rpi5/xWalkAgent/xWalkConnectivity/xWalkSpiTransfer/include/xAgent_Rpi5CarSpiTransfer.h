/******************************************************************************
 * @file        xAgent_Rpi5CarSpiTransfer.h
 * @brief       Declares the bounded xWalk SPI transfer Agent.
 *
 * @details
 * Coordinates caller-supplied SPI request bytes with one caller-owned HAL SPI
 * interface while retaining no platform resource or device configuration.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpiTransfer
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

#ifndef XAGENT_RPI5CAR_SPI_TRANSFER_H
#define XAGENT_RPI5CAR_SPI_TRANSFER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpi.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkSpiTransfer
     * @brief Coordinates one bounded synchronous SPI transaction at a time.
     *
     * @details
     * Observes one non-null caller-owned `XWalkSpi`. The SPI interface and its
     * backend must outlive this Agent. External serialization is required.
     */
    class XWalkSpiTransfer final
    {
        private:
            /** @brief Non-owning non-null SPI pointer that is never released. */
            hal::XWalkSpi* spiObject{nullptr};

        public:
            /**
             * @brief Binds one caller-owned SPI interface.
             * @param[in] spi SPI interface that must outlive this Agent.
             */
            explicit XWalkSpiTransfer(hal::XWalkSpi& spi) noexcept;

            /** @brief Releases no caller-owned SPI resource. */
            ~XWalkSpiTransfer();

            XWalkSpiTransfer(const XWalkSpiTransfer&) = delete;
            XWalkSpiTransfer& operator=(const XWalkSpiTransfer&) = delete;
            XWalkSpiTransfer(XWalkSpiTransfer&&) = delete;
            XWalkSpiTransfer& operator=(XWalkSpiTransfer&&) = delete;

            /**
             * @brief Executes one bounded full-duplex transaction.
             * @param[in] transmitData Non-empty payload containing at most 256 bytes.
             * @return Received bytes with exactly the transmitted length.
             */
            agent::bytevector transfer(const agent::bytevector& transmitData);
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_SPI_TRANSFER_H */
