/******************************************************************************
 * @file        xHal_Rpi5CarRgbLedLifecycle.cpp
 * @brief       Implements RGB LED construction and destruction.
 *
 * @details
 * Stores non-owning pointers to caller-created PWM channels and validates the
 * selected common-terminal configuration.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarRgbLed.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs an RGB LED controller from three PWM outputs.
 *
 * @param[in] red
 * Non-owning PWM output connected to the red LED channel.
 *
 * @param[in] green
 * Non-owning PWM output connected to the green LED channel.
 *
 * @param[in] blue
 * Non-owning PWM output connected to the blue LED channel.
 *
 * @param[in] common
 * Shared terminal configuration; common-anode is the Python-compatible default.
 *
 * @pre
 * All three PWM objects outlive this controller.
 *
 * @throws std::invalid_argument
 * If `common` contains an unsupported underlying value.
 */
XWalkRgbLed::XWalkRgbLed(XWalkPwm& red, XWalkPwm& green, XWalkPwm& blue,
    XWalkRgbLedCommon common):
    redPwm(&red), greenPwm(&green), bluePwm(&blue), commonValue(validateCommon(common))
{
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the RGB LED controller.
 *
 * @note
 * The three PWM dependency pointers are non-owning and are not released.
 */
XWalkRgbLed::~XWalkRgbLed() = default;

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates a common-terminal selector.
 *
 * @param[in] common
 * Common-cathode or common-anode selector.
 *
 * @return
 * The validated selector.
 *
 * @throws std::invalid_argument
 * If the enumeration contains an unsupported underlying value.
 */
XWalkRgbLedCommon XWalkRgbLed::validateCommon(XWalkRgbLedCommon common)
{
    const uint8 commonSelection = static_cast<uint8>(common);
    const boolean isCathode = commonSelection == XHAL_RPI5CAR_RGB_LED_COMMON_CATHODE;
    const boolean isAnode = commonSelection == XHAL_RPI5CAR_RGB_LED_COMMON_ANODE;
    if ((!isCathode) && (!isAnode))
    {
        XHAL_THROW_INVALID_ARGUMENT("RGB LED common mode must be cathode or anode");
    }
    return common;
}

} /* namespace xwalk::hal */
