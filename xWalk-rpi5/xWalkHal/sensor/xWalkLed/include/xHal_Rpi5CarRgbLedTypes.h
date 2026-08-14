/******************************************************************************
 * @file        xHal_Rpi5CarRgbLedTypes.h
 * @brief       Declares RGB LED connection and color types.
 *
 * @details
 * Defines the common-terminal configuration and fixed red, green, and blue
 * component representation used by the RGB LED controller.
 *
 * @project     xWalk Firmware
 * @module      xWalkLed
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

#ifndef XHAL_RPI5CAR_RGB_LED_TYPES_H
#define XHAL_RPI5CAR_RGB_LED_TYPES_H

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
 * Type definitions
 ******************************************************************************/

/** @brief Fixed red, green, and blue components in the inclusive range 0 to 255. */
using rgbcolor = fixedarray<uint8, XHAL_RPI5CAR_RGB_LED_CHANNEL_COUNT>;

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/**
 * @brief Identifies the shared electrical terminal of an RGB LED.
 */
enum class XWalkRgbLedCommon : uint8
{
    Cathode = XHAL_RPI5CAR_RGB_LED_COMMON_CATHODE, /**< Components drive active-high outputs. */
    Anode = XHAL_RPI5CAR_RGB_LED_COMMON_ANODE      /**< Components drive inverted outputs. */
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_RGB_LED_TYPES_H */
