/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiSpiTransfer.cpp
 * @brief       Composes the isolated Raspberry Pi SPI-transfer mode.
 * @details     Opens only the configured spidev endpoint for one synchronous callback.
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

#include "xAgent_Rpi5CarSpiTransfer.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarSpiLinux.h"

namespace xwalk::agent
{

    /**
     * @brief Runs one configured SPI transaction service.
     * @param[in,out] applicationContext Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runSpiTransfer(agent::contextpointer applicationContext,
                                              bootapplicationcallback callback,
                                              hal::XWalkConfigStore& config)
    {
        const agent::string spiDevice = config.get("hardware_spi_device", XHAL_RPI5CAR_SPI_DEFAULT_DEVICE);
        hal::XWalkSpiConfiguration spiConfiguration{};
        spiConfiguration.speedHz = parseUnsigned(
            config.get("hardware_spi_speed_hz", "500000"), "hardware_spi_speed_hz", hal::common::UINT32_MAXIMUM);
        spiConfiguration.mode = static_cast<agent::uint8>(
            parseUnsigned(config.get("hardware_spi_mode", "0"), "hardware_spi_mode", XHAL_RPI5CAR_SPI_MAXIMUM_MODE));
        spiConfiguration.bitsPerWord =
            static_cast<agent::uint8>(parseUnsigned(config.get("hardware_spi_bits_per_word", "8"),
                                                    "hardware_spi_bits_per_word",
                                                    XHAL_RPI5CAR_SPI_MAXIMUM_BITS_PER_WORD));
        hal::XWalkSpiLinux spiBackend(spiDevice.c_str(), spiConfiguration);
        hal::XWalkSpi spi(&spiBackend, XHAL_SPI_TRANSFER_CALLBACK(hal::XWalkSpiLinux));
        XWalkSpiTransfer spiTransfer(spi);
        XWalkBootServices services{};
        services.spiTransfer = &spiTransfer;
        return callback(applicationContext, services);
    }

} /* namespace xwalk::agent */
