/******************************************************************************
 * @file        xHal_Rpi5CarDeviceTypes.h
 * @brief       Declares Robot HAT model and device-information types.
 *
 * @details
 * Defines the detected hardware revision and the product metadata and board
 * configuration derived from Linux firmware device-tree properties.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl
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

#ifndef XHAL_RPI5CAR_DEVICE_TYPES_H
#define XHAL_RPI5CAR_DEVICE_TYPES_H

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
     * Enumeration declarations
     ******************************************************************************/

    /** @brief Identifies the recognized Robot HAT hardware revision. */
    enum class XWalkDeviceModel : uint8
    {
        Unknown = 0U,    /**< No supported UUID was found. */
        RobotHatV4 = 1U, /**< Default legacy configuration without a detected UUID. */
        RobotHatV5 = 2U  /**< Robot HAT v5 identified by its device-tree UUID. */
    };

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Contains detected HAT metadata and board-specific configuration. */
    struct XWalkDeviceInformation
    {
            /** @brief Product text read from the `product` device-tree property. */
            string productName{};
            /** @brief Unsigned hexadecimal value read from the `product_id` property. */
            uint32 productId{};
            /** @brief Unsigned hexadecimal value read from the `product_ver` property. */
            uint32 productVersion{};
            /** @brief UUID text used to recognize the supported HAT revision. */
            string uuid{};
            /** @brief Vendor text read from the `vendor` device-tree property. */
            string vendor{};
            /** @brief GPIO line offset controlling physical speaker enable. */
            uint8 speakerEnablePin{XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN};
            /** @brief Board-specific motor-driver mode identifier. */
            uint8 motorMode{XHAL_RPI5CAR_DEVICE_DEFAULT_MOTOR_MODE};
            /** @brief Effective board configuration, defaulting to legacy Robot HAT v4. */
            XWalkDeviceModel model{XWalkDeviceModel::RobotHatV4};
            /** @brief `true` when a supported HAT UUID and all required properties were read. */
            boolean detected{};
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_DEVICE_TYPES_H */
