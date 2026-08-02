/******************************************************************************
 * @file        xHal_Rpi5CarMotor.h
 * @brief       Declares the Robot HAT single-motor control interface.
 *
 * @details
 * Defines both supported motor-driver modes, signed speed control, reversal,
 * and braking using caller-owned PWM and GPIO dependencies.
 *
 * @project     xWalk Firmware
 * @module      xWalkMotor
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

#ifndef XHAL_RPI5CAR_MOTOR_H
#define XHAL_RPI5CAR_MOTOR_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarPwm.h"

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

/** @brief Identifies the Robot HAT motor-driver wiring strategy. */
enum class XWalkMotorMode : uint8
{
    PwmAndDirection = 1U, /**< TC1508S-style PWM speed plus digital direction control. */
    DualPwm = 2U         /**< TC618S-style independent forward and reverse PWM inputs. */
};

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkMotor
 * @brief Controls one Robot HAT DC motor using one of two driver modes.
 *
 * @details
 * Stores non-owning pointers to caller-created PWM and GPIO objects. Signed speed selects direction, while
 * its absolute magnitude controls duty cycle in the range 0.0 through 100.0 percent.
 */
class XWalkMotor
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Non-owning pointer to the primary speed-control PWM channel.
         *
         * @note
         * Never null after construction and must outlive this motor.
         */
        XWalkPwm* pwmA{nullptr};

        /**
         * @brief Nullable non-owning pointer to the reverse PWM channel in dual-PWM mode.
         *
         * @note
         * Null in PWM-and-direction mode; otherwise the object must outlive this motor.
         */
        XWalkPwm* pwmB{nullptr};

        /**
         * @brief Nullable non-owning pointer to the digital direction pin.
         *
         * @note
         * Non-null only in PWM-and-direction mode and must outlive this motor.
         */
        XWalkGpio* directionPin{nullptr};

        /** @brief Selected motor-driver mode established by the constructor overload. */
        XWalkMotorMode modeValue{XWalkMotorMode::PwmAndDirection};

        /** @brief Last successfully applied signed speed in the range -100.0 to 100.0 percent. */
        float64 speedPercentValue{};

        /** @brief Configured PWM frequency in Hertz. */
        float64 frequencyHzValue{XHAL_RPI5CAR_MOTOR_DEFAULT_FREQUENCY_HZ};

        /** @brief Reverses the logical forward direction when `true`. */
        boolean reversedValue{false};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Validates a signed motor speed command.
         *
         * @param[in] speedPercent
         * Signed speed in the inclusive range -100.0 to 100.0 percent.
         *
         * @return
         * Validated speed command.
         *
         * @throws std::invalid_argument
         * If the value is not finite.
         *
         * @throws std::out_of_range
         * If the value is outside the permitted range.
         */
        static float64 validateSpeed(float64 speedPercent);

        /**
         * @brief Validates a motor PWM frequency.
         *
         * @param[in] frequencyHz
         * Finite frequency greater than zero, in Hertz.
         *
         * @return
         * Validated frequency in Hertz.
         *
         * @throws std::invalid_argument
         * If the frequency is non-finite or not greater than zero.
         */
        static float64 validateFrequency(float64 frequencyHz);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a PWM-and-direction motor driver.
         *
         * @param[in] pwm
         * Primary PWM dependency that must outlive this motor.
         *
         * @param[in] direction
         * Digital direction dependency that must outlive this motor.
         *
         * @param[in] reversed
         * `true` to exchange the logical forward and reverse directions.
         *
         * @param[in] frequencyHz
         * Finite PWM frequency greater than zero, in Hertz.
         *
         * @post
         * The PWM channel uses `frequencyHz` and zero-percent duty cycle.
         *
         * @throws std::invalid_argument
         * If `frequencyHz` is non-finite or not greater than zero.
         */
        XWalkMotor(XWalkPwm& pwm, XWalkGpio& direction, boolean reversed = false,
            float64 frequencyHz = XHAL_RPI5CAR_MOTOR_DEFAULT_FREQUENCY_HZ);

        /**
         * @brief Constructs a dual-PWM motor driver.
         *
         * @param[in] forwardPwm
         * Forward PWM dependency that must outlive this motor.
         *
         * @param[in] reversePwm
         * Reverse PWM dependency that must outlive this motor.
         *
         * @param[in] reversed
         * `true` to exchange the logical forward and reverse directions.
         *
         * @param[in] frequencyHz
         * Finite PWM frequency greater than zero, in Hertz.
         *
         * @post
         * Both PWM channels use `frequencyHz` and zero-percent duty cycle.
         *
         * @throws std::invalid_argument
         * If `frequencyHz` is non-finite or not greater than zero.
         */
        XWalkMotor(XWalkPwm& forwardPwm, XWalkPwm& reversePwm, boolean reversed = false,
            float64 frequencyHz = XHAL_RPI5CAR_MOTOR_DEFAULT_FREQUENCY_HZ);

        /** @brief Makes one non-throwing stop attempt and releases no non-owning dependency. */
        ~XWalkMotor() noexcept;

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction to preserve dependency identity. */
        XWalkMotor(XWalkMotor&&) = delete;
        /** @brief Disables copying of dependency bindings. */
        XWalkMotor(const XWalkMotor&) = delete;
        /** @brief Disables move assignment of dependency bindings. */
        XWalkMotor& operator=(XWalkMotor&&) = delete;
        /** @brief Disables copy assignment of dependency bindings. */
        XWalkMotor& operator=(const XWalkMotor&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Applies a signed motor speed command.
         *
         * @param[in] speedPercent
         * Signed speed in the inclusive range -100.0 to 100.0 percent.
         *
         * @post
         * Direction outputs and PWM duty cycles represent the validated command.
         *
         * @throws std::invalid_argument
         * If the value is not finite.
         *
         * @throws std::out_of_range
         * If the value is outside the permitted range.
         */
        void setSpeed(float64 speedPercent);

        /**
         * @brief Commands a zero-percent duty cycle without changing reversal configuration.
         *
         * @post
         * `speed()` returns zero and all active speed PWM outputs have zero-percent duty cycle.
         */
        void stop();

        /**
         * @brief Makes a non-throwing best-effort attempt to disable every speed PWM output.
         *
         * @return
         * `true` when every required PWM output accepted zero percent; otherwise `false`.
         *
         * @post
         * Every required PWM output has received an independent zero-percent attempt. `speed()` returns zero
         * only when all required outputs accepted the request.
         *
         * @note
         * This operation is reserved for scope-bound cleanup and emergency-stop paths.
         */
        boolean stopSafely() noexcept;

        /**
         * @brief Electrically brakes a dual-PWM motor.
         *
         * @throws std::invalid_argument
         * If this object uses PWM-and-direction mode.
         */
        void brake();

        /**
         * @brief Configures logical direction reversal.
         *
         * @param[in] reversed
         * `true` to exchange logical forward and reverse.
         *
         * @post
         * Future speed commands use the requested direction mapping.
         */
        void setReversed(boolean reversed) noexcept;

        /**
         * @brief Returns the last successfully applied signed speed.
         *
         * @return
         * Speed in the inclusive range -100.0 to 100.0 percent.
         */
        float64 speed() const noexcept;

        /**
         * @brief Returns whether logical direction is reversed.
         *
         * @return
         * `true` when forward and reverse are exchanged; otherwise `false`.
         */
        boolean reversed() const noexcept;

        /**
         * @brief Returns the motor-driver mode.
         *
         * @return
         * Mode selected by the constructor overload.
         */
        XWalkMotorMode mode() const noexcept;

        /**
         * @brief Returns the configured PWM frequency.
         *
         * @return
         * Requested motor frequency in Hertz.
         */
        float64 frequency() const noexcept;
};


} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_MOTOR_H */
