/******************************************************************************
 * @file        xHal_Rpi5CarMotorLifecycle.cpp
 * @brief       Implements single-motor validation and lifecycle behavior.
 *
 * @details
 * Validates speed and frequency inputs, binds caller-owned hardware, and
 * provides explicit initialization for the selected motor-driver outputs.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMath.h"
#include "xHal_Rpi5CarMotor.h"
#include "xHal_Rpi5CarTrace.h"

#include <memory>

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
     * Protected member function definitions
     ******************************************************************************/

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
    float64 XWalkMotor::validateSpeed(float64 speedPercent)
    {
        const hal::boolean speedPercentNotFinite = static_cast<hal::boolean>(!XHAL_IS_FINITE(speedPercent));
        if (speedPercentNotFinite)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "motor speed must be finite");
        }
        if ((speedPercent < XHAL_RPI5CAR_MOTOR_MIN_SPEED_PERCENT) ||
            (speedPercent > XHAL_RPI5CAR_MOTOR_MAX_SPEED_PERCENT))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "motor speed must be between -100 and 100 percent");
        }
        return speedPercent;
    }

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
    float64 XWalkMotor::validateFrequency(float64 frequencyHz)
    {
        const hal::boolean frequencyHzInvalid =
            static_cast<hal::boolean>((!XHAL_IS_FINITE(frequencyHz)) || (frequencyHz <= 0.0));
        if (frequencyHzInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "motor frequency must be finite and greater than zero");
        }
        return frequencyHz;
    }

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

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
     * No PWM or GPIO operation has occurred.
     */
    XWalkMotor::XWalkMotor(XWalkPwm& pwm, XWalkGpio& direction, boolean reversed, float64 frequencyHz)
        : pwmA(&pwm), directionPin(&direction), modeValue(XWalkMotorMode::PwmAndDirection),
          frequencyHzValue(validateFrequency(frequencyHz)), reversedValue(reversed)
    {
        XWALK_HAL_TRACE_UID1(
            RPI .245, "Uninitialized PWM-and-direction motor constructed at %.2f Hz", frequencyHzValue);
    }

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
     * No PWM operation has occurred.
     */
    XWalkMotor::XWalkMotor(XWalkPwm& forwardPwm, XWalkPwm& reversePwm, boolean reversed, float64 frequencyHz)
        : pwmA(&forwardPwm), pwmB(&reversePwm), modeValue(XWalkMotorMode::DualPwm),
          frequencyHzValue(validateFrequency(frequencyHz)), reversedValue(reversed)
    {
        XWALK_HAL_TRACE_UID1(RPI .246, "Uninitialized dual-PWM motor constructed at %.2f Hz", frequencyHzValue);
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Makes one non-throwing stop attempt and releases no non-owning
     * dependency.
     */
    XWalkMotor::~XWalkMotor() noexcept
    {
        static_cast<void>(stopSafely());
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Explicitly configures the motor outputs and establishes zero duty
     * cycle.
     *
     * @return
     * `true` after successful initialization, including an idempotent repeated
     * call.
     *
     * @post
     * On success, every PWM dependency uses the configured frequency and zero duty
     * cycle. On failure, every PWM dependency receives an independent best-effort
     * zero request and the motor remains uninitialized.
     */
    boolean XWalkMotor::initialize()
    {
        if (initializedValue)
        {
            return true;
        }
        const auto rollbackOperation = [this](void*) noexcept
        {
            static_cast<void>(pwmA->trySetPulseWidthPercent(0.0));
            if (pwmB != nullptr)
            {
                static_cast<void>(pwmB->trySetPulseWidthPercent(0.0));
            }
            initializedValue = false;
        };
        std::unique_ptr<void, decltype(rollbackOperation)> rollbackGuard(this, rollbackOperation);
        pwmA->setFrequency(frequencyHzValue);
        pwmA->setPulseWidthPercent(0.0);
        if (pwmB != nullptr)
        {
            pwmB->setFrequency(frequencyHzValue);
            pwmB->setPulseWidthPercent(0.0);
        }
        speedPercentValue = 0.0;
        initializedValue = true;
        static_cast<void>(rollbackGuard.release());
        return true;
    }

} /* namespace xwalk::hal */
