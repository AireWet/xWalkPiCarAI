/******************************************************************************
 * @file        xHal_Rpi5CarPwm.h
 * @brief       Declares the Robot HAT PWM channel abstraction.
 *
 * @details
 * Provides channel selection, timer configuration, duty-cycle control, and
 * register output through a non-owning hardware-independent I2C interface.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm
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

#ifndef XHAL_RPI5CAR_PWM_H
#define XHAL_RPI5CAR_PWM_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"

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
 * @class XWalkPwm
 * @brief Controls one Robot HAT PWM output channel.
 *
 * @details
 * Maps channels zero through nineteen to shared hardware timers, selects or
 * probes the Robot HAT I2C address, calculates timer settings from requested
 * frequencies, and writes 16-bit output values in high-byte-first order.
 */
class XWalkPwm
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Non-owning pointer to the I2C interface used for all writes.
         *
         * @note
         * The pointer is initialized from a constructor reference, is never
         * null after construction, and must outlive this PWM object.
         */
        XWalkI2c* i2cObject;

        /**
         * @brief Non-owning pointer to the shared seven-timer period state.
         *
         * @note
         * The pointer is initialized from a constructor reference, is never
         * null after construction, and must outlive this PWM object.
         */
        pwmtimerstatepointer timerState;

        /** @brief Selected seven-bit Robot HAT I2C address. */
        uint8 addressValue{};
        /** @brief PWM channel index in the inclusive range 0 to 19. */
        uint32 channelValue{};
        /** @brief Shared hardware timer index in the inclusive range 0 to 6. */
        uint32 timerIndexValue{};
        /** @brief Last requested pulse width in 16-bit timer-count units. */
        uint32 pulseWidthValue{};
        /** @brief Active prescaler value in the inclusive range 1 to 65536. */
        uint32 prescalerValue{1U};
        /** @brief Last requested duty cycle in the range 0.0 to 100.0 percent. */
        float64 pulseWidthPercentValue{};
        /** @brief Requested or derived PWM frequency in Hertz. */
        float64 frequencyHzValue{XHAL_RPI5CAR_PWM_DEFAULT_FREQUENCY_HZ};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Writes an unsigned 16-bit value to a Robot HAT register.
         *
         * @param[in] reg
         * Eight-bit destination register address.
         *
         * @param[in] value
         * Register value in the inclusive range 0 to 65535.
         *
         * @post
         * The I2C interface receives a two-byte high-byte-first payload.
         *
         * @throws std::out_of_range
         * If `value` exceeds 16 bits.
         */
        void write16(uint8 reg, uint32 value);

        /**
         * @brief Selects an explicit or automatically probed I2C address.
         *
         * @param[in] requested
         * Optional explicit seven-bit address. Any supplied byte is returned
         * without probing; an omitted address triggers probing of `0x14` through
         * `0x16`.
         *
         * @return
         * The explicit address, the first responding candidate, or `0x14` when
         * no candidate responds.
         */
        uint8 selectAddress(optionaluint8 requested);

        /**
         * @brief Maps a PWM channel to its shared hardware timer.
         *
         * @param[in] channel
         * PWM channel in the inclusive range 0 to 19.
         *
         * @return
         * Timer index in the inclusive range 0 to 6.
         */
        static uint32 timerForChannel(uint32 channel);

        /**
         * @brief Parses a textual PWM channel designation.
         *
         * @param[in] channel
         * Channel text in the form `P0` through `P19`.
         *
         * @return
         * Parsed channel index in the inclusive range 0 to 19.
         *
         * @throws std::invalid_argument
         * If the text does not begin with `P` followed by decimal digits.
         *
         * @throws std::out_of_range
         * If the parsed index exceeds 19.
         */
        static uint32 parseChannel(stringview channel);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a PWM channel from a numeric channel index.
         *
         * @param[in] i2c
         * Non-owning I2C interface used for probing and register writes.
         *
         * @param[in] channel
         * PWM channel in the inclusive range 0 to 19.
         *
         * @param[in] address
         * Optional explicit seven-bit I2C address; omission enables probing.
         *
         * @param[in] sharedTimerState
         * Shared timer state created by the caller and passed by reference.
         *
         * @pre
         * `i2c` and `sharedTimerState` outlive this object.
         *
         * @post
         * The selected channel is initialized to a requested frequency of 50 Hz.
         *
         * @throws std::out_of_range
         * If `channel` exceeds 19, `address` exceeds the seven-bit range, or the
         * default frequency is not representable.
         */
        XWalkPwm(XWalkI2c& i2c, uint32 channel, optionaluint8 address,
            XWalkPwmTimerState& sharedTimerState);

        /**
         * @brief Constructs a PWM channel from a textual channel designation.
         *
         * @param[in] i2c
         * Non-owning I2C interface used for probing and register writes.
         *
         * @param[in] channel
         * Channel text in the form `P0` through `P19`.
         *
         * @param[in] address
         * Optional explicit seven-bit I2C address; omission enables probing.
         *
         * @param[in] sharedTimerState
         * Shared timer state created by the caller and passed by reference.
         *
         * @pre
         * `i2c` and `sharedTimerState` outlive this object.
         *
         * @post
         * Successful construction produces the same state as the numeric
         * overload for the parsed channel.
         *
         * @throws std::invalid_argument
         * If the channel text is malformed.
         *
         * @throws std::out_of_range
         * If the parsed channel exceeds 19, `address` exceeds the seven-bit
         * range, or the default frequency is not representable.
         */
        XWalkPwm(XWalkI2c& i2c, stringview channel, optionaluint8 address,
            XWalkPwmTimerState& sharedTimerState);

        /** @brief Destroys the PWM object without owning its collaborators. */
        ~XWalkPwm();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction to preserve dependency identity. */
        XWalkPwm(XWalkPwm&&) = delete;
        /** @brief Disables copying of non-owning dependency bindings. */
        XWalkPwm(const XWalkPwm&) = delete;
        /** @brief Disables move assignment of dependency bindings. */
        XWalkPwm& operator=(XWalkPwm&&) = delete;
        /** @brief Disables copy assignment of dependency bindings. */
        XWalkPwm& operator=(const XWalkPwm&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Configures the shared timer period.
         *
         * @param[in] periodValue
         * Requested period in timer-count units; rounded valid range is 1 to
         * 65535.
         *
         * @post
         * The shared period, derived frequency, and hardware period register
         * reflect the rounded value.
         *
         * @throws std::invalid_argument
         * If `periodValue` is non-finite.
         *
         * @throws std::out_of_range
         * If the rounded value is outside 1 to 65535.
         */
        void setPeriod(float64 periodValue);

        /**
         * @brief Configures the shared timer prescaler.
         *
         * @param[in] prescaler
         * Requested division factor; rounded valid range is 1 to 65536.
         *
         * @post
         * The stored prescaler, derived frequency, and hardware prescaler
         * register reflect the rounded value.
         *
         * @throws std::invalid_argument
         * If `prescaler` is non-finite.
         *
         * @throws std::out_of_range
         * If the rounded value is outside 1 to 65536.
         */
        void setPrescaler(float64 prescaler);

        /**
         * @brief Configures the channel pulse width.
         *
         * @param[in] pulseWidth
         * Pulse width in timer-count units; valid range is 0.0 to 65535.0.
         *
         * @post
         * The converted timer count is stored and written to the channel.
         *
         * @throws std::invalid_argument
         * If `pulseWidth` is non-finite.
         *
         * @throws std::out_of_range
         * If `pulseWidth` is outside 0.0 to 65535.0.
         */
        void setPulseWidth(float64 pulseWidth);

        /**
         * @brief Calculates and configures timer settings for a frequency.
         *
         * @param[in] frequencyHz
         * Finite requested output frequency greater than zero, in Hertz.
         *
         * @post
         * The closest representable settings found by the local search are
         * written to the shared timer registers.
         *
         * @throws std::invalid_argument
         * If `frequencyHz` is non-finite, not greater than zero, or truncates
         * to zero.
         *
         * @throws std::out_of_range
         * If `frequencyHz` exceeds the unsigned 32-bit range or no valid timer
         * period is found.
         */
        void setFrequency(float64 frequencyHz);

        /**
         * @brief Configures the pulse width as a percentage of the shared period.
         *
         * @param[in] percent
         * Finite duty cycle in the inclusive range 0.0 to 100.0 percent.
         *
         * @post
         * The requested percentage is stored and its truncated pulse width is
         * written to the channel.
         *
         * @throws std::invalid_argument
         * If `percent` is non-finite.
         *
         * @throws std::out_of_range
         * If `percent` is outside 0.0 to 100.0 percent.
         */
        void setPulseWidthPercent(float64 percent);

        /**
         * @brief Attempts to set duty cycle through the non-throwing I2C path.
         *
         * @param[in] percent
         * Finite duty cycle in the inclusive range 0.0 to 100.0 percent.
         *
         * @return
         * `true` when zero through one hundred percent is written; otherwise `false`.
         *
         * @post
         * The stored pulse width and percentage change only after a successful write.
         */
        boolean trySetPulseWidthPercent(float64 percent) noexcept;

        /**
         * @brief Returns the selected Robot HAT I2C address.
         *
         * @return
         * Selected seven-bit I2C address.
         */
        uint8 address() const noexcept { return addressValue; }

        /**
         * @brief Returns the current shared timer period.
         *
         * @return
         * Current period in timer-count units.
         */
        uint32 period() const;

        /**
         * @brief Returns the selected PWM channel index.
         *
         * @return
         * PWM channel index in the inclusive range 0 to 19.
         */
        uint32 channel() const noexcept { return channelValue; }

        /**
         * @brief Returns the shared hardware timer index.
         *
         * @return
         * Timer index in the inclusive range 0 to 6.
         */
        uint32 timerIndex() const noexcept { return timerIndexValue; }

        /**
         * @brief Returns the active timer prescaler.
         *
         * @return
         * Prescaler in the inclusive range 1 to 65536.
         */
        uint32 prescaler() const noexcept { return prescalerValue; }

        /**
         * @brief Returns the last requested pulse width.
         *
         * @return
         * Pulse width in 16-bit timer-count units.
         */
        uint32 pulseWidth() const noexcept { return pulseWidthValue; }

        /**
         * @brief Returns the requested or derived output frequency.
         *
         * @return
         * Output frequency in Hertz.
         */
        float64 frequency() const noexcept { return frequencyHzValue; }

        /**
         * @brief Returns the last requested duty cycle.
         *
         * @return
         * Duty cycle as a percentage.
         */
        float64 pulseWidthPercent() const noexcept
        {
            return pulseWidthPercentValue;
        }
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_PWM_H */
