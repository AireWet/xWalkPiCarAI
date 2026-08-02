/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpi.h
 * @brief       Declares the Raspberry Pi xWalk hardware composition owner.
 *
 * @details
 * Selects one bounded hardware graph and retains it for one synchronous
 * application callback before deterministic reverse-order destruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
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

#ifndef XAGENT_RPI5CAR_BOOT_RPI_H
#define XAGENT_RPI5CAR_BOOT_RPI_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBoot.h"

namespace xwalk::hal
{
struct XWalkDeviceInformation;
}

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkBootRpi
 * @brief Owns one complete command-specific Raspberry Pi boot lifetime.
 */
class XWalkBootRpi final : private XWalkBoot
{
    private:
        /** @brief Hardware graph selected for this process operation. */
        XWalkBootMode selectedMode{XWalkBootMode::Base};
        /** @brief Owned writable PiCar-X configuration path. */
        hal::string configurationFilePath{};
        /** @brief Suspends one Agent action without application dependencies. */
        static void delayMilliseconds(hal::contextpointer context, hal::uint32 durationMs);
        /** @brief Suspends one self-drive action and reports completion. */
        static hal::boolean selfDriveDelayMilliseconds(hal::contextpointer context,
            hal::uint32 durationMs) noexcept;
        /** @brief Accepts speaker priming without emitting an audible sample. */
        static void primeSpeaker(hal::contextpointer context, hal::uint32 durationMs);
        /** @brief Parses one bounded unsigned decimal deployment value. */
        static hal::uint32 parseUnsigned(hal::stringview value,
            hal::stringview optionName, hal::uint32 maximum);
        /** @brief Applies fail-safe automatic or explicit Robot HAT selection. */
        static hal::XWalkDeviceInformation selectBoard(
            const hal::XWalkDeviceInformation& detectedInformation,
            hal::stringview requestedBoard);

    public:
        /**
         * @brief Stores one boot selection without claiming hardware.
         * @param[in] mode Minimum hardware graph required by the application command.
         * @param[in] configFilePath Non-empty writable PiCar-X configuration path.
         * @throws std::invalid_argument If `configFilePath` is empty.
         */
        XWalkBootRpi(XWalkBootMode mode, hal::stringview configFilePath);

        /** @brief Releases retained boot state after the process operation. */
        ~XWalkBootRpi() = default;

        XWalkBootRpi(XWalkBootRpi&&) = delete;
        XWalkBootRpi(const XWalkBootRpi&) = delete;
        XWalkBootRpi& operator=(XWalkBootRpi&&) = delete;
        XWalkBootRpi& operator=(const XWalkBootRpi&) = delete;

        /**
         * @brief Claims hardware and executes one application callback.
         * @param[in,out] context Nullable caller-owned application context.
         * @param[in] callback Non-null callback completed before hardware teardown.
         * @return Status returned by `callback`.
         * @throws std::invalid_argument If `callback` is null.
         * @throws std::logic_error If this object already started once.
         * @warning Claims physical I2C, GPIO, motor, servo, and optional audio resources.
         */
        hal::int32 run(hal::contextpointer context, bootapplicationcallback callback);
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_BOOT_RPI_H */
