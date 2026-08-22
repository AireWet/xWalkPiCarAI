/******************************************************************************
 * @file        xHal_Rpi5CarLogicalModels.cpp
 * @brief       Implements deterministic logical Robot HAT behavioral models.
 * @project     xWalk Firmware
 * @module      xWalkRobotHatSimulation
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xHal_Rpi5CarLogicalModels.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace xwalk::hal::simulation
{

    namespace
    {
        /** @brief Records one event without allowing unbounded growth. */
        void recordEvent(XWalkLogicalModelState& state,
                         XWalkLogicalModelEventId identifier,
                         float64 firstValue = 0.0,
                         float64 secondValue = 0.0) noexcept
        {
            const size eventCapacity = state.events.size();
            const uint64 maximumDroppedEvents = std::numeric_limits<uint64>::max();
            if (state.eventCount < eventCapacity)
            {
                state.events[state.eventCount] = {
                    state.nextEventSequence, state.logicalTime, identifier, firstValue, secondValue};
                ++state.eventCount;
            }
            else if (state.droppedEvents != maximumDroppedEvents)
            {
                ++state.droppedEvents;
            }
            const uint64 maximumEventSequence = std::numeric_limits<uint64>::max();
            if (state.nextEventSequence != maximumEventSequence)
            {
                ++state.nextEventSequence;
            }
        }

        /** @brief Clamps one finite percentage to the supported motor range. */
        float64 clampSpeed(float64 speed) noexcept
        {
            return std::clamp(speed, -100.0, 100.0);
        }

        /** @brief Moves current toward target by a bounded positive delta. */
        float64 moveToward(float64 current, float64 target, float64 increase, float64 decrease) noexcept
        {
            const float64 limit = std::abs(target) > std::abs(current) ? increase : decrease;
            if (target > current)
            {
                return std::min(target, current + limit);
            }
            return std::max(target, current - limit);
        }

        /** @brief Reports common finite and positive rate validation. */
        boolean positiveFinite(float64 value) noexcept
        {
            return std::isfinite(value) && (value > 0.0);
        }
    } /* namespace */

    XWalkLogicalModelStatus
    validateLogicalModelConfiguration(const XWalkLogicalModelConfiguration& configuration) noexcept
    {
        const boolean configurationInvalid =
            !positiveFinite(configuration.accelerationPerTick) || !positiveFinite(configuration.decelerationPerTick) ||
            !std::isfinite(configuration.steeringMinimum) || !std::isfinite(configuration.steeringMaximum) ||
            !std::isfinite(configuration.servoCentre) || !positiveFinite(configuration.servoTravel) ||
            (configuration.steeringMinimum >= configuration.steeringMaximum) ||
            (configuration.servoCentre - configuration.servoTravel < configuration.steeringMinimum) ||
            (configuration.servoCentre + configuration.servoTravel > configuration.steeringMaximum) ||
            !positiveFinite(configuration.initialBatteryVoltage) ||
            !positiveFinite(configuration.batteryWarningVoltage) ||
            !positiveFinite(configuration.batteryCriticalVoltage) ||
            (configuration.initialBatteryVoltage < configuration.batteryWarningVoltage) ||
            (configuration.batteryWarningVoltage <= configuration.batteryCriticalVoltage) ||
            !std::isfinite(configuration.batteryReductionPerTick) || (configuration.batteryReductionPerTick < 0.0) ||
            (configuration.grayscaleSequence.size() > XWALK_LOGICAL_MODEL_MAXIMUM_SEQUENCE_VALUES) ||
            (configuration.ultrasonicSequence.size() > XWALK_LOGICAL_MODEL_MAXIMUM_SEQUENCE_VALUES);
        if (configurationInvalid)
        {
            return XWalkLogicalModelStatus::InvalidConfiguration;
        }
        for (const std::array<uint16, 3U>& sample : configuration.grayscaleSequence)
        {
            if ((sample[0U] > 4'095U) || (sample[1U] > 4'095U) || (sample[2U] > 4'095U))
            {
                return XWalkLogicalModelStatus::InvalidConfiguration;
            }
        }
        for (float64 distance : configuration.ultrasonicSequence)
        {
            const boolean distanceInvalid = !std::isfinite(distance) || (distance < 0.0) || (distance > 500.0);
            if (distanceInvalid)
            {
                return XWalkLogicalModelStatus::InvalidConfiguration;
            }
        }
        return XWalkLogicalModelStatus::Ok;
    }

    XWalkLogicalModelStatus initializeLogicalModel(XWalkLogicalModelState& state,
                                                   const XWalkLogicalModelConfiguration& configuration) noexcept
    {
        state = {};
        const XWalkLogicalModelStatus validationStatus = validateLogicalModelConfiguration(configuration);
        if (validationStatus != XWalkLogicalModelStatus::Ok)
        {
            return XWalkLogicalModelStatus::InvalidConfiguration;
        }
        state.configuration = configuration;
        state.steeringAngle = configuration.servoCentre;
        state.batteryVoltage = configuration.initialBatteryVoltage;
        state.initialized = true;
        state.armed = true;
        recordEvent(state, XWalkLogicalModelEventId::Initialized);
        return XWalkLogicalModelStatus::Ok;
    }

    XWalkLogicalModelStatus
    commandLogicalMotors(XWalkLogicalModelState& state, float64 leftSpeed, float64 rightSpeed) noexcept
    {
        if (!state.initialized)
        {
            return XWalkLogicalModelStatus::NotInitialized;
        }
        const boolean speedInvalid = !std::isfinite(leftSpeed) || !std::isfinite(rightSpeed);
        if (speedInvalid)
        {
            enterLogicalSafeState(state);
            return XWalkLogicalModelStatus::InvalidConfiguration;
        }
        state.commandedLeftSpeed = clampSpeed(state.configuration.invertLeftDirection ? -leftSpeed : leftSpeed);
        state.commandedRightSpeed = clampSpeed(state.configuration.invertRightDirection ? -rightSpeed : rightSpeed);
        recordEvent(state, XWalkLogicalModelEventId::MotorCommand, state.commandedLeftSpeed, state.commandedRightSpeed);
        return XWalkLogicalModelStatus::Ok;
    }

    XWalkLogicalModelStatus advanceLogicalModel(XWalkLogicalModelState& state, uint64 ticks) noexcept
    {
        if (!state.initialized)
        {
            return XWalkLogicalModelStatus::NotInitialized;
        }
        for (uint64 tick = 0U; tick < ticks; ++tick)
        {
            const uint64 maximumLogicalTime = std::numeric_limits<uint64>::max();
            if (state.logicalTime != maximumLogicalTime)
            {
                ++state.logicalTime;
            }
            state.simulatedLeftSpeed = moveToward(state.simulatedLeftSpeed,
                                                  state.commandedLeftSpeed,
                                                  state.configuration.accelerationPerTick,
                                                  state.configuration.decelerationPerTick);
            state.simulatedRightSpeed = moveToward(state.simulatedRightSpeed,
                                                   state.commandedRightSpeed,
                                                   state.configuration.accelerationPerTick,
                                                   state.configuration.decelerationPerTick);
            state.batteryVoltage = std::max(0.0, state.batteryVoltage - state.configuration.batteryReductionPerTick);
            if (!state.batteryWarning && (state.batteryVoltage <= state.configuration.batteryWarningVoltage))
            {
                state.batteryWarning = true;
                recordEvent(state, XWalkLogicalModelEventId::BatteryWarning, state.batteryVoltage);
            }
            if (!state.batteryCritical && (state.batteryVoltage <= state.configuration.batteryCriticalVoltage))
            {
                state.batteryCritical = true;
                recordEvent(state, XWalkLogicalModelEventId::BatteryCritical, state.batteryVoltage);
                enterLogicalSafeState(state);
            }
        }
        recordEvent(state, XWalkLogicalModelEventId::MotorState, state.simulatedLeftSpeed, state.simulatedRightSpeed);
        return XWalkLogicalModelStatus::Ok;
    }

    XWalkLogicalModelStatus commandLogicalSteering(XWalkLogicalModelState& state, float64 angle) noexcept
    {
        if (!state.initialized)
        {
            return XWalkLogicalModelStatus::NotInitialized;
        }
        const boolean angleFinite = std::isfinite(angle);
        if (!angleFinite)
        {
            enterLogicalSafeState(state);
            return XWalkLogicalModelStatus::InvalidConfiguration;
        }
        const float64 minimum = state.configuration.servoCentre - state.configuration.servoTravel;
        const float64 maximum = state.configuration.servoCentre + state.configuration.servoTravel;
        state.steeringAngle = std::clamp(angle, minimum, maximum);
        recordEvent(state, XWalkLogicalModelEventId::SteeringState, state.steeringAngle);
        return XWalkLogicalModelStatus::Ok;
    }

    XWalkLogicalModelStatus nextLogicalGrayscale(XWalkLogicalModelState& state, std::array<uint16, 3U>& sample) noexcept
    {
        if (!state.initialized)
        {
            return XWalkLogicalModelStatus::NotInitialized;
        }
        const size grayscaleCount = state.configuration.grayscaleSequence.size();
        if (state.grayscaleIndex >= grayscaleCount)
        {
            sample = {};
            return XWalkLogicalModelStatus::EndOfSequence;
        }
        sample = state.configuration.grayscaleSequence[state.grayscaleIndex];
        ++state.grayscaleIndex;
        recordEvent(state, XWalkLogicalModelEventId::GrayscaleSample, sample[0U], sample[2U]);
        return XWalkLogicalModelStatus::Ok;
    }

    XWalkLogicalModelStatus nextLogicalUltrasonic(XWalkLogicalModelState& state, float64& distanceCentimeters) noexcept
    {
        if (!state.initialized)
        {
            return XWalkLogicalModelStatus::NotInitialized;
        }
        const size ultrasonicCount = state.configuration.ultrasonicSequence.size();
        if (state.ultrasonicIndex >= ultrasonicCount)
        {
            distanceCentimeters = 0.0;
            return XWalkLogicalModelStatus::EndOfSequence;
        }
        distanceCentimeters = state.configuration.ultrasonicSequence[state.ultrasonicIndex];
        ++state.ultrasonicIndex;
        recordEvent(state, XWalkLogicalModelEventId::UltrasonicSample, distanceCentimeters);
        return XWalkLogicalModelStatus::Ok;
    }

    XWalkLogicalModelStatus nextLogicalCameraFrame(XWalkLogicalModelState& state, uint64& frameIdentifier) noexcept
    {
        if (!state.initialized)
        {
            return XWalkLogicalModelStatus::NotInitialized;
        }
        const uint64 maximumLogicalTime = std::numeric_limits<uint64>::max();
        if (maximumLogicalTime - state.logicalTime < state.configuration.cameraDelayTicks)
        {
            state.logicalTime = std::numeric_limits<uint64>::max();
        }
        else
        {
            state.logicalTime += state.configuration.cameraDelayTicks;
        }
        const uint64 maximumFrameIdentifier = std::numeric_limits<uint64>::max();
        if (!state.configuration.freezeCamera && (state.cameraFrameIdentifier != maximumFrameIdentifier))
        {
            ++state.cameraFrameIdentifier;
        }
        frameIdentifier = state.cameraFrameIdentifier;
        recordEvent(state,
                    state.configuration.freezeCamera ? XWalkLogicalModelEventId::CameraFrozenFrame
                                                     : XWalkLogicalModelEventId::CameraFrame,
                    static_cast<float64>(frameIdentifier));
        return XWalkLogicalModelStatus::Ok;
    }

    boolean logicalI2cOperationFails(XWalkLogicalModelState& state) noexcept
    {
        if (!state.initialized)
        {
            return true;
        }
        const uint32 maximumOperationCount = std::numeric_limits<uint32>::max();
        if (state.i2cOperationCount != maximumOperationCount)
        {
            ++state.i2cOperationCount;
        }
        const boolean failed = (state.configuration.i2cFailureInterval != 0U) &&
                               ((state.i2cOperationCount % state.configuration.i2cFailureInterval) == 0U);
        if (failed)
        {
            recordEvent(state, XWalkLogicalModelEventId::I2cFailure, state.i2cOperationCount);
            enterLogicalSafeState(state);
        }
        return failed;
    }

    void enterLogicalSafeState(XWalkLogicalModelState& state) noexcept
    {
        state.commandedLeftSpeed = 0.0;
        state.commandedRightSpeed = 0.0;
        state.simulatedLeftSpeed = 0.0;
        state.simulatedRightSpeed = 0.0;
        state.armed = false;
        recordEvent(state, XWalkLogicalModelEventId::SafeState);
    }

} /* namespace xwalk::hal::simulation */
