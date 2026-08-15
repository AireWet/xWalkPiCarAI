/******************************************************************************
 * @file        xHal_Rpi5CarRgbLed.cpp
 * @brief       Implements RGB color decoding and PWM output.
 *
 * @details
 * Converts component, packed, and hexadecimal color representations to PWM
 * percentages while respecting common-cathode or common-anode polarity.
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

#include "xHal_Rpi5CarTrace.h"

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
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Sets the logical RGB color from individual components.
     *
     * @param[in] color
     * Red, green, and blue values in the inclusive range 0 to 255.
     *
     * @post
     * The PWM outputs reflect the requested color and configured polarity if no
     * output fails.
     */
    void XWalkRgbLed::setColor(const rgbcolor& color)
    {
        applyColor(color);
    }

    /**
     * @brief Sets the logical RGB color from packed data.
     *
     * @param[in] packedColor
     * Color encoded as `0xRRGGBB`, in the inclusive range `0x000000` to `0xFFFFFF`.
     *
     * @throws std::out_of_range
     * If bits outside the least-significant 24 bits are set.
     */
    void XWalkRgbLed::setColor(uint32 packedColor)
    {
        applyColor(decodeColor(packedColor));
    }

    /**
     * @brief Sets the logical RGB color from hexadecimal text.
     *
     * @param[in] hexadecimalColor
     * Six hexadecimal digits, optionally surrounded by `#` characters.
     *
     * @throws std::invalid_argument
     * If the stripped input is not exactly six hexadecimal digits.
     */
    void XWalkRgbLed::setColor(stringview hexadecimalColor)
    {
        applyColor(decodeColor(parseHexColor(hexadecimalColor)));
    }

    /**
     * @brief Returns the last logical color applied successfully.
     *
     * @return
     * Immutable red, green, and blue component array owned by this object.
     */
    const rgbcolor& XWalkRgbLed::color() const noexcept
    {
        return colorValue;
    }

    /**
     * @brief Returns the configured common-terminal mode.
     *
     * @return
     * Common-cathode or common-anode connection mode.
     */
    XWalkRgbLedCommon XWalkRgbLed::common() const noexcept
    {
        return commonValue;
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Decodes a packed color into red, green, and blue components.
     *
     * @param[in] packedColor
     * Color encoded as `0xRRGGBB`, in the inclusive range `0x000000` to `0xFFFFFF`.
     *
     * @return
     * Components ordered red, green, and blue.
     *
     * @throws std::out_of_range
     * If bits outside the least-significant 24 bits are set.
     */
    rgbcolor XWalkRgbLed::decodeColor(uint32 packedColor)
    {
        if (packedColor > XHAL_RPI5CAR_RGB_LED_MAX_PACKED_COLOR)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "RGB LED packed color must be in range 0x000000..0xFFFFFF");
        }

        const uint32 redValue = (packedColor & XHAL_RPI5CAR_RGB_LED_RED_MASK) >> XHAL_RPI5CAR_RGB_LED_RED_SHIFT;
        const uint32 greenValue = (packedColor & XHAL_RPI5CAR_RGB_LED_GREEN_MASK) >> XHAL_RPI5CAR_RGB_LED_GREEN_SHIFT;
        const uint32 blueValue = (packedColor & XHAL_RPI5CAR_RGB_LED_BLUE_MASK) >> XHAL_RPI5CAR_RGB_LED_BLUE_SHIFT;
        return {static_cast<uint8>(redValue), static_cast<uint8>(greenValue), static_cast<uint8>(blueValue)};
    }

    /**
     * @brief Parses hexadecimal text into packed `0xRRGGBB` data.
     *
     * @param[in] hexadecimalColor
     * Six hexadecimal digits, optionally surrounded by `#` characters.
     *
     * @return
     * Packed color in the inclusive range `0x000000` to `0xFFFFFF`.
     *
     * @throws std::invalid_argument
     * If the stripped input is not exactly six hexadecimal digits.
     */
    uint32 XWalkRgbLed::parseHexColor(stringview hexadecimalColor)
    {
        size firstDigit = 0U;
        size endPosition = hexadecimalColor.size();
        while ((firstDigit != endPosition) && (hexadecimalColor[firstDigit] == '#'))
        {
            ++firstDigit;
        }
        while ((endPosition > firstDigit) && (hexadecimalColor[endPosition - 1U] == '#'))
        {
            --endPosition;
        }

        const size digitCount = endPosition - firstDigit;
        if (digitCount != XHAL_RPI5CAR_RGB_LED_HEX_DIGIT_COUNT)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "RGB LED color text must contain six hexadecimal digits");
        }

        uint32 packedColor = 0U;
        for (size index = firstDigit; index < endPosition; ++index)
        {
            const uint32 digitValue = static_cast<uint32>(hexDigitValue(hexadecimalColor[index]));
            packedColor = (packedColor << 4U) | digitValue;
        }
        return packedColor;
    }

    /**
     * @brief Converts one hexadecimal character to its numeric nibble.
     *
     * @param[in] digit
     * Character in the ranges `0` to `9`, `a` to `f`, or `A` to `F`.
     *
     * @return
     * Numeric value in the inclusive range 0 to 15.
     *
     * @throws std::invalid_argument
     * If `digit` is not hexadecimal.
     */
    uint8 XWalkRgbLed::hexDigitValue(char digit)
    {
        uint8 value = 0U;
        if ((digit >= '0') && (digit <= '9'))
        {
            value = static_cast<uint8>(digit - '0');
        }
        else if ((digit >= 'a') && (digit <= 'f'))
        {
            value = static_cast<uint8>((digit - 'a') + 10);
        }
        else if ((digit >= 'A') && (digit <= 'F'))
        {
            value = static_cast<uint8>((digit - 'A') + 10);
        }
        else
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "RGB LED color text contains a non-hexadecimal character");
        }
        return value;
    }

    /**
     * @brief Converts one logical color component to PWM duty-cycle percent.
     *
     * @param[in] component
     * Eight-bit logical intensity in the inclusive range 0 to 255.
     *
     * @return
     * Duty cycle in the inclusive range 0.0 to 100.0 percent, inverted for
     * common-anode wiring.
     */
    float64 XWalkRgbLed::componentToPercent(uint8 component) const noexcept
    {
        uint32 outputComponent = static_cast<uint32>(component);
        if (commonValue == XWalkRgbLedCommon::Anode)
        {
            outputComponent = XHAL_RPI5CAR_RGB_LED_MAX_COMPONENT - outputComponent;
        }

        const float64 outputValue = static_cast<float64>(outputComponent);
        const float64 maximumValue = static_cast<float64>(XHAL_RPI5CAR_RGB_LED_MAX_COMPONENT);
        const float64 normalizedValue = outputValue / maximumValue;
        return normalizedValue * XHAL_RPI5CAR_RGB_LED_PERCENT_SCALE;
    }

    /**
     * @brief Applies one validated logical color to all PWM outputs.
     *
     * @param[in] color
     * Components ordered red, green, and blue, each in the range 0 to 255.
     *
     * @post
     * All three PWM outputs and the stored logical color reflect the request if no
     * output fails.
     */
    void XWalkRgbLed::applyColor(const rgbcolor& color)
    {
        const float64 redPercent = componentToPercent(color[XHAL_RPI5CAR_RGB_LED_RED_CHANNEL]);
        const float64 greenPercent = componentToPercent(color[XHAL_RPI5CAR_RGB_LED_GREEN_CHANNEL]);
        const float64 bluePercent = componentToPercent(color[XHAL_RPI5CAR_RGB_LED_BLUE_CHANNEL]);

        redPwm->setPulseWidthPercent(redPercent);
        greenPwm->setPulseWidthPercent(greenPercent);
        bluePwm->setPulseWidthPercent(bluePercent);
        colorValue = color;
        XWALK_HAL_TRACE_UID3(RPI .266,
                             "RGB LED color applied as red %u, green %u, and blue %u",
                             static_cast<uint32>(color[XHAL_RPI5CAR_RGB_LED_RED_CHANNEL]),
                             static_cast<uint32>(color[XHAL_RPI5CAR_RGB_LED_GREEN_CHANNEL]),
                             static_cast<uint32>(color[XHAL_RPI5CAR_RGB_LED_BLUE_CHANNEL]));
    }

} /* namespace xwalk::hal */
