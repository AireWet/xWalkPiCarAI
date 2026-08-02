/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorLinux.h
 * @brief       Declares passive Linux deployment inspection.
 *
 * @details
 * Reports Raspberry Pi and Robot HAT readiness without claiming GPIO lines,
 * resetting the MCU, moving actuators, capturing media, or contacting services.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi Doctor
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

#ifndef XAGENT_RPI5CAR_DOCTOR_LINUX_H
#define XAGENT_RPI5CAR_DOCTOR_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

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
 * @class XWalkDoctorLinux
 * @brief Performs one passive Linux deployment inspection.
 *
 * @details
 * Uses descriptor metadata, read-only filesystem inspection, passive device
 * opens, firmware reads, and an ADC battery sample. It never requests a GPIO
 * line, performs an SPI transfer, enables audio, captures media, or contacts a
 * language-model endpoint.
 */
class XWalkDoctorLinux final
{
    protected:
        /** @brief Reads one flat configuration value without modifying its file. */
        static hal::string configurationValue(hal::stringview filePath,
            hal::stringview key, hal::stringview fallback);
        /** @brief Appends one consistently formatted report result. */
        static void appendResult(hal::stringvector& lines, hal::boolean passed,
            hal::stringview name, hal::stringview detail);
        /** @brief Appends one non-failing advisory result. */
        static void appendWarning(hal::stringvector& lines,
            hal::stringview name, hal::stringview detail);
        /** @brief Reports whether one regular path is readable. */
        static hal::boolean readablePath(hal::stringview path);
        /** @brief Reports whether one named or absolute executable is available. */
        static hal::boolean executableAvailable(hal::stringview executable);
        /** @brief Reports whether one shared library can be loaded without retaining it. */
        static hal::boolean libraryAvailable(hal::stringview libraryName);
        /** @brief Reads a short property file without throwing on I/O failure. */
        static hal::string readProperty(hal::stringview path);
        /** @brief Locates the supported Robot HAT v5 UUID below one Device Tree root. */
        static hal::boolean robotHatV5Detected(hal::stringview root);
        /** @brief Inspects one GPIO chip without requesting a line. */
        static void inspectGpio(hal::stringvector& lines, hal::stringview device,
            hal::stringview expectedName, hal::stringview expectedLabel);
        /** @brief Reads Robot HAT firmware and battery data without constructing actuators. */
        static void inspectI2c(hal::stringvector& lines, hal::stringview device);
        /** @brief Checks one SPI device by opening and closing it without transferring data. */
        static void inspectSpi(hal::stringvector& lines, hal::stringview device);
        /** @brief Checks passive camera, audio, model, executable, and library prerequisites. */
        static void inspectOptionalServices(hal::stringvector& lines,
            hal::stringview configurationFilePath);

    public:
        /**
         * @brief Builds one passive deployment report.
         *
         * @param[in] configurationFilePath
         * Non-empty flat deployment configuration path inspected without mutation.
         *
         * @return
         * Owned report lines prefixed with `[PASS]`, `[WARN]`, or `[FAIL]`.
         */
        static hal::stringvector inspect(hal::stringview configurationFilePath);
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_DOCTOR_LINUX_H */
