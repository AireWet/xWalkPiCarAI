/******************************************************************************
 * @file        xHal_Rpi5CarRgbLed.h
 * @brief       Declares the three-channel Robot HAT RGB LED controller.
 *
 * @details
 * Provides color decoding, common-terminal polarity handling, and PWM output
 * through three caller-owned xWalk PWM channel objects.
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

#ifndef XHAL_RPI5CAR_RGB_LED_H
#define XHAL_RPI5CAR_RGB_LED_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarRgbLedTypes.h"

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
 * @class XWalkRgbLed
 * @brief Controls one RGB LED using three independent PWM outputs.
 *
 * @details
 * Accepts component arrays, packed `0xRRGGBB` values, or six-digit hexadecimal
 * text. Output polarity is adjusted for common-cathode or common-anode wiring.
 */
class XWalkRgbLed
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Non-owning pointer to the PWM output driving the red channel.
         *
         * @note Initialized from a constructor reference, never null, and must outlive this object.
         */
        XWalkPwm* redPwm;

        /**
         * @brief Non-owning pointer to the PWM output driving the green channel.
         *
         * @note Initialized from a constructor reference, never null, and must outlive this object.
         */
        XWalkPwm* greenPwm;

        /**
         * @brief Non-owning pointer to the PWM output driving the blue channel.
         *
         * @note Initialized from a constructor reference, never null, and must outlive this object.
         */
        XWalkPwm* bluePwm;

        /** @brief Electrical common-terminal configuration used for output polarity. */
        XWalkRgbLedCommon commonValue;
        /** @brief Last logical red, green, and blue color applied successfully. */
        rgbcolor colorValue{};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

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
        static XWalkRgbLedCommon validateCommon(XWalkRgbLedCommon common);

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
        static rgbcolor decodeColor(uint32 packedColor);

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
        static uint32 parseHexColor(stringview hexadecimalColor);

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
        static uint8 hexDigitValue(char digit);

        /**
         * @brief Converts one logical color component to PWM duty-cycle percent.
         *
         * @param[in] component
         * Eight-bit logical intensity in the inclusive range 0 to 255.
         *
         * @return
         * Duty cycle in the inclusive range 0.0 to 100.0 percent, inverted for common-anode wiring.
         */
        float64 componentToPercent(uint8 component) const noexcept;

        /**
         * @brief Applies one validated logical color to all PWM outputs.
         *
         * @param[in] color
         * Components ordered red, green, and blue, each in the range 0 to 255.
         *
         * @post
         * All three PWM outputs and the stored logical color reflect the request if no output fails.
         */
        void applyColor(const rgbcolor& color);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

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
        XWalkRgbLed(XWalkPwm& red, XWalkPwm& green, XWalkPwm& blue,
            XWalkRgbLedCommon common = XWalkRgbLedCommon::Anode);

        /** @brief Destroys the controller without releasing its non-owning PWM dependencies. */
        ~XWalkRgbLed();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction to preserve dependency identity. */
        XWalkRgbLed(XWalkRgbLed&&) = delete;
        /** @brief Disables copying of non-owning dependency bindings. */
        XWalkRgbLed(const XWalkRgbLed&) = delete;
        /** @brief Disables move assignment of dependency bindings. */
        XWalkRgbLed& operator=(XWalkRgbLed&&) = delete;
        /** @brief Disables copy assignment of dependency bindings. */
        XWalkRgbLed& operator=(const XWalkRgbLed&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Sets the logical RGB color from individual components.
         *
         * @param[in] color
         * Red, green, and blue values in the inclusive range 0 to 255.
         *
         * @post
         * The PWM outputs reflect the requested color and configured polarity if no output fails.
         */
        void setColor(const rgbcolor& color);

        /**
         * @brief Sets the logical RGB color from packed data.
         *
         * @param[in] packedColor
         * Color encoded as `0xRRGGBB`, in the inclusive range `0x000000` to `0xFFFFFF`.
         *
         * @throws std::out_of_range
         * If bits outside the least-significant 24 bits are set.
         */
        void setColor(uint32 packedColor);

        /**
         * @brief Sets the logical RGB color from hexadecimal text.
         *
         * @param[in] hexadecimalColor
         * Six hexadecimal digits, optionally surrounded by `#` characters.
         *
         * @throws std::invalid_argument
         * If the stripped input is not exactly six hexadecimal digits.
         */
        void setColor(stringview hexadecimalColor);

        /**
         * @brief Returns the last logical color applied successfully.
         *
         * @return
         * Immutable red, green, and blue component array owned by this object.
         */
        const rgbcolor& color() const noexcept;

        /**
         * @brief Returns the configured common-terminal mode.
         *
         * @return
         * Common-cathode or common-anode connection mode.
         */
        XWalkRgbLedCommon common() const noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_RGB_LED_H */
