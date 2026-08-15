/******************************************************************************
 * @file        xHal_Rpi5CarMotorsLifecycle.cpp
 * @brief       Implements paired-motor validation and lifecycle behavior.
 *
 * @details
 * Validates motor identifiers and speed pairs, binds two caller-owned motors,
 * applies role configuration, and preserves non-owning dependency lifetimes.
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
#include "xHal_Rpi5CarMotors.h"
#include "xHal_Rpi5CarTrace.h"

#include <chrono>

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
    uint8 XWalkMotors::validateMotorId(uint8 motorId)
    {
        if ((motorId != XHAL_RPI5CAR_MOTOR_FIRST_ID) && (motorId != XHAL_RPI5CAR_MOTOR_SECOND_ID))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "motor identifier must be 1 or 2");
        }
        return motorId;
    }

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
    void XWalkMotors::validateSpeeds(float64 leftSpeedPercent, float64 rightSpeedPercent)
    {
        const hal::boolean leftSpeedPercentRightSpeedPercentInvalid =
            static_cast<hal::boolean>((!XHAL_IS_FINITE(leftSpeedPercent)) || (!XHAL_IS_FINITE(rightSpeedPercent)));
        if (leftSpeedPercentRightSpeedPercentInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "paired motor speeds must be finite");
        }
        const boolean leftOutsideRange = (leftSpeedPercent < XHAL_RPI5CAR_MOTOR_MIN_SPEED_PERCENT) ||
                                         (leftSpeedPercent > XHAL_RPI5CAR_MOTOR_MAX_SPEED_PERCENT);
        const boolean rightOutsideRange = (rightSpeedPercent < XHAL_RPI5CAR_MOTOR_MIN_SPEED_PERCENT) ||
                                          (rightSpeedPercent > XHAL_RPI5CAR_MOTOR_MAX_SPEED_PERCENT);
        if (leftOutsideRange || rightOutsideRange)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "paired motor speeds must be between -100 and 100 percent");
        }
    }

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

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
     * @post
     * Both role identifiers are valid and their configured reversal states are
     * applied.
     */
    XWalkMotors::XWalkMotors(XWalkMotor& firstMotor,
                             XWalkMotor& secondMotor,
                             const XWalkMotorsConfiguration& configuration)
        : motorOne(&firstMotor), motorTwo(&secondMotor), configurationValue(configuration)
    {
        configurationValue.leftMotorId = validateMotorId(configuration.leftMotorId);
        configurationValue.rightMotorId = validateMotorId(configuration.rightMotorId);
        if (configurationValue.leftMotorId == configurationValue.rightMotorId)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "motor role assignments must be different");
        }
        if ((configurationValue.watchdogTimeoutMilliseconds == 0U) ||
            (configurationValue.watchdogTimeoutMilliseconds > XHAL_RPI5CAR_MOTOR_WATCHDOG_MAXIMUM_MILLISECONDS))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "motor.watchdog_timeout_ms must be from 1 through 60000");
        }
        left().setReversed(configurationValue.leftReversed);
        right().setReversed(configurationValue.rightReversed);
        if (configurationValue.watchdogWorkerEnabled)
        {
            if (configurationValue.beforeThreadStart != nullptr)
            {
                configurationValue.beforeThreadStart(configurationValue.threadStartContext);
            }
            watchdogThread = std::thread(&XWalkMotors::watchdogWorker, this);
        }
        XWALK_HAL_TRACE_UID0(RPI .250, "Paired motor controller constructed");
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Makes one non-throwing paired stop attempt and releases no motor
     * pointer.
     */
    XWalkMotors::~XWalkMotors() noexcept
    {
        {
            const std::lock_guard<std::mutex> lock(safetyMutex);
            shutdownRequestedValue = true;
            armedValue = false;
            commandActiveValue = false;
        }
        watchdogCondition.notify_all();
        if (watchdogThread.joinable())
        {
            watchdogThread.join();
        }
        static_cast<void>(stopSafely());
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    uint64 XWalkMotors::nowMilliseconds() const noexcept
    {
        if (configurationValue.clockMilliseconds != nullptr)
        {
            return configurationValue.clockMilliseconds(configurationValue.clockContext);
        }
        const auto elapsed = std::chrono::steady_clock::now().time_since_epoch();
        return static_cast<uint64>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
    }

    boolean XWalkMotors::stopSafelyUnlocked() noexcept
    {
        const boolean firstStopped = motorOne->stopSafely();
        const boolean secondStopped = motorTwo->stopSafely();
        return firstStopped && secondStopped;
    }

    void XWalkMotors::refreshWatchdogUnlocked() noexcept
    {
        commandRefreshMillisecondsValue = nowMilliseconds();
    }

    void XWalkMotors::watchdogWorker() noexcept
    {
        std::unique_lock<std::mutex> lock(safetyMutex);
        while (shutdownRequestedValue == false)
        {
            watchdogCondition.wait_for(lock, std::chrono::milliseconds(configurationValue.watchdogTimeoutMilliseconds));
            if (shutdownRequestedValue)
            {
                break;
            }
            if (armedValue && commandActiveValue)
            {
                const uint64 currentMilliseconds = nowMilliseconds();
                const boolean clockRolledBack = currentMilliseconds < commandRefreshMillisecondsValue;
                const boolean timeoutExpired =
                    !clockRolledBack && ((currentMilliseconds - commandRefreshMillisecondsValue) >=
                                         static_cast<uint64>(configurationValue.watchdogTimeoutMilliseconds));
                if (clockRolledBack || timeoutExpired)
                {
                    armedValue = false;
                    commandActiveValue = false;
                    static_cast<void>(stopSafelyUnlocked());
                }
            }
        }
    }

} /* namespace xwalk::hal */
