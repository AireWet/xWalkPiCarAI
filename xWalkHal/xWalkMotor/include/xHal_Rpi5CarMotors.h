/******************************************************************************
 * @file        xHal_Rpi5CarMotors.h
 * @brief       Declares coordinated left and right motor control.
 *
 * @details
 * Defines paired-motor role configuration and movement operations using two
 * caller-owned XWalkMotor dependencies.
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

#ifndef XHAL_RPI5CAR_MOTORS_H
#define XHAL_RPI5CAR_MOTORS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMotor.h"

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

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @brief Contains caller-owned paired-motor role and reversal configuration.
 *
 * @details
 * Persistence is intentionally external. A future configuration component may load and save this value.
 */
struct XWalkMotorsConfiguration
{
    uint8 leftMotorId{XHAL_RPI5CAR_MOTOR_FIRST_ID}; /**< One-based motor assigned to the left side. */
    uint8 rightMotorId{XHAL_RPI5CAR_MOTOR_SECOND_ID}; /**< One-based motor assigned to the right side. */
    boolean leftReversed{false}; /**< Reverses the configured left motor when `true`. */
    boolean rightReversed{false}; /**< Reverses the configured right motor when `true`. */
};

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkMotors
 * @brief Coordinates two caller-owned motors as left and right drive units.
 *
 * @details
 * Stores two non-owning motor pointers, applies explicit role configuration, and provides vehicle movement
 * commands without owning PWM, GPIO, motor, or persistent configuration resources.
 */
class XWalkMotors
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning pointer to one caller-created motor; never null. */
        XWalkMotor* motorOne{nullptr};

        /** @brief Non-owning pointer to the other caller-created motor; never null. */
        XWalkMotor* motorTwo{nullptr};

        /** @brief Runtime role and reversal configuration supplied by the caller. */
        XWalkMotorsConfiguration configurationValue{};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Validates a one-based motor identifier.
         *
         * @param[in] motorId
         * Identifier expected to be 1 or 2.
         *
         * @return
         * Validated identifier.
         *
         * @throws std::out_of_range
         * If the identifier is not 1 or 2.
         */
        static uint8 validateMotorId(uint8 motorId);

        /**
         * @brief Validates both signed speed commands before changing either motor.
         *
         * @param[in] leftSpeedPercent
         * Left speed in the inclusive range -100.0 to 100.0 percent.
         *
         * @param[in] rightSpeedPercent
         * Right speed in the inclusive range -100.0 to 100.0 percent.
         *
         * @throws std::invalid_argument
         * If either value is non-finite.
         *
         * @throws std::out_of_range
         * If either value is outside the permitted range.
         */
        static void validateSpeeds(float64 leftSpeedPercent, float64 rightSpeedPercent);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a paired-motor controller.
         *
         * @param[in] firstMotor
         * First motor dependency that must outlive this controller.
         *
         * @param[in] secondMotor
         * Second motor dependency that must outlive this controller.
         *
         * @param[in] configuration
         * Runtime role and reversal configuration.
         *
         * @throws std::out_of_range
         * If either motor identifier is not 1 or 2.
         */
        XWalkMotors(XWalkMotor& firstMotor, XWalkMotor& secondMotor,
            const XWalkMotorsConfiguration& configuration = {});

        /** @brief Makes one non-throwing paired stop attempt and releases no motor pointer. */
        ~XWalkMotors() noexcept;

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction to preserve motor identity. */
        XWalkMotors(XWalkMotors&&) = delete;
        /** @brief Disables copying of motor dependency bindings. */
        XWalkMotors(const XWalkMotors&) = delete;
        /** @brief Disables move assignment of motor dependency bindings. */
        XWalkMotors& operator=(XWalkMotors&&) = delete;
        /** @brief Disables copy assignment of motor dependency bindings. */
        XWalkMotors& operator=(const XWalkMotors&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Stops both configured drive motors.
         *
         * @post
         * Both motors contain a zero-percent signed speed command.
         */
        void stop();

        /**
         * @brief Makes independent non-throwing stop attempts for both drive motors.
         *
         * @return
         * `true` when both motors accepted every required zero-output request; otherwise `false`.
         *
         * @post
         * Both configured motors have received a stop attempt even if either attempt fails.
         *
         * @note
         * This operation is reserved for scope-bound cleanup and emergency-stop paths.
         */
        boolean stopSafely() noexcept;

        /**
         * @brief Brakes both configured drive motors when both support braking.
         *
         * @throws std::invalid_argument
         * If either motor is not configured for dual-PWM mode.
         */
        void brake();

        /**
         * @brief Applies independent signed left and right speed commands.
         *
         * @param[in] leftSpeedPercent
         * Left speed in the inclusive range -100.0 to 100.0 percent.
         *
         * @param[in] rightSpeedPercent
         * Right speed in the inclusive range -100.0 to 100.0 percent.
         *
         * @throws std::invalid_argument
         * If either value is non-finite.
         *
         * @throws std::out_of_range
         * If either value is outside the permitted range.
         */
        void setSpeed(float64 leftSpeedPercent, float64 rightSpeedPercent);

        /**
         * @brief Drives both motors forward at the supplied magnitude.
         *
         * @param[in] speedPercent
         * Signed speed in the range -100.0 to 100.0 percent.
         */
        void forward(float64 speedPercent);

        /**
         * @brief Drives both motors backward at the supplied magnitude.
         *
         * @param[in] speedPercent
         * Signed speed in the range -100.0 to 100.0 percent; the command negates this value.
         */
        void backward(float64 speedPercent);

        /**
         * @brief Commands a counter-rotating left turn.
         *
         * @param[in] speedPercent
         * Signed speed in the range -100.0 to 100.0 percent.
         */
        void turnLeft(float64 speedPercent);

        /**
         * @brief Commands a counter-rotating right turn.
         *
         * @param[in] speedPercent
         * Signed speed in the range -100.0 to 100.0 percent.
         */
        void turnRight(float64 speedPercent);

        /**
         * @brief Assigns motor 1 or 2 to the left side.
         *
         * @param[in] motorId
         * One-based motor identifier, either 1 or 2.
         */
        void setLeftMotorId(uint8 motorId);

        /**
         * @brief Assigns motor 1 or 2 to the right side.
         *
         * @param[in] motorId
         * One-based motor identifier, either 1 or 2.
         */
        void setRightMotorId(uint8 motorId);

        /**
         * @brief Sets and applies the left motor reversal state.
         *
         * @param[in] reversed
         * Requested left motor reversal state.
         */
        void setLeftReversed(boolean reversed) noexcept;

        /**
         * @brief Sets and applies the right motor reversal state.
         *
         * @param[in] reversed
         * Requested right motor reversal state.
         */
        void setRightReversed(boolean reversed) noexcept;

        /**
         * @brief Toggles the left motor reversal state.
         *
         * @return
         * Updated left motor reversal state.
         */
        boolean toggleLeftReversed() noexcept;

        /**
         * @brief Toggles the right motor reversal state.
         *
         * @return
         * Updated right motor reversal state.
         */
        boolean toggleRightReversed() noexcept;

        /**
         * @brief Returns the motor assigned to the left side.
         *
         * @return
         * Non-owning reference to the configured left motor.
         */
        XWalkMotor& left();

        /**
         * @brief Returns the motor assigned to the right side.
         *
         * @return
         * Non-owning reference to the configured right motor.
         */
        XWalkMotor& right();

        /**
         * @brief Returns motor 1 or 2 by one-based identifier.
         *
         * @param[in] motorId
         * One-based motor identifier, either 1 or 2.
         *
         * @return
         * Non-owning reference to the selected motor.
         */
        XWalkMotor& motor(uint8 motorId);

        /**
         * @brief Returns a copy of the runtime configuration for external persistence.
         *
         * @return
         * Current role identifiers and reversal states.
         */
        XWalkMotorsConfiguration configuration() const noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_MOTORS_H */
