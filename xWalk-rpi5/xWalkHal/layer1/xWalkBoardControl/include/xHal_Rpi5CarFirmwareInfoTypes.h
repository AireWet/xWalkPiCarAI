/******************************************************************************
 * @file        xHal_Rpi5CarFirmwareInfoTypes.h
 * @brief       Declares the Robot HAT firmware-version value type.
 *
 * @details
 * Defines the fixed major, minor, and patch representation returned by the
 * xWalk firmware-information reader.
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

#ifndef XHAL_RPI5CAR_FIRMWARE_INFO_TYPES_H
#define XHAL_RPI5CAR_FIRMWARE_INFO_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

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
     * Structure declarations
     ******************************************************************************/

    /** @brief Contains the three unsigned components of a Robot HAT firmware version. */
    struct XWalkFirmwareVersion
    {
            /** @brief Major version reported by the first register byte. */
            uint8 major{};
            /** @brief Minor version reported by the second register byte. */
            uint8 minor{};
            /** @brief Patch version reported by the third register byte. */
            uint8 patch{};
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_FIRMWARE_INFO_TYPES_H */
