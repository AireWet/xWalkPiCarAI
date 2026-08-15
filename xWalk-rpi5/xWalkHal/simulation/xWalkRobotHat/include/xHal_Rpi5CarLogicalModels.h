/******************************************************************************
 * @file        xHal_Rpi5CarLogicalModels.h
 * @brief       Declares deterministic logical Robot HAT behavioral models.
 * @project     xWalk Firmware
 * @module      xWalkRobotHatSimulation
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_LOGICAL_MODELS_H
#define XHAL_RPI5CAR_LOGICAL_MODELS_H

#include "xHal_Rpi5CarTypes.h"

#include <array>
#include <vector>

namespace xwalk::hal::simulation
{

    constexpr size XWALK_LOGICAL_MODEL_MAXIMUM_SEQUENCE_VALUES{1'024U};
    constexpr size XWALK_LOGICAL_MODEL_MAXIMUM_EVENTS{512U};

    /** @brief Reports one logical model operation result. */
    enum class XWalkLogicalModelStatus : uint8
    {
        Ok = 0U,
        InvalidConfiguration,
        NotInitialized,
        EndOfSequence
    };

    /** @brief Identifies a stable logical-model event. */
    enum class XWalkLogicalModelEventId : uint8
    {
        Initialized = 0U,
        MotorCommand,
        MotorState,
        SteeringState,
        BatteryWarning,
        BatteryCritical,
        GrayscaleSample,
        UltrasonicSample,
        CameraFrame,
        CameraFrozenFrame,
        I2cFailure,
        SafeState
    };

    /** @brief Stores one ordered bounded logical-model event. */
    struct XWalkLogicalModelEvent
    {
            uint64 sequence{};    /**< Monotonic sequence beginning at one. */
            uint64 logicalTime{}; /**< Caller-controlled logical timestamp. */
            XWalkLogicalModelEventId identifier{XWalkLogicalModelEventId::Initialized};
            float64 firstValue{};  /**< Event-specific primary value. */
            float64 secondValue{}; /**< Event-specific secondary value. */
    };

    /** @brief Configures bounded logical behavior, not physical vehicle dynamics. */
    struct XWalkLogicalModelConfiguration
    {
            float64 accelerationPerTick{5.0};     /**< Maximum speed increase per logical tick. */
            float64 decelerationPerTick{10.0};    /**< Maximum speed decrease per logical tick. */
            boolean invertLeftDirection{};        /**< Reverses left command sign. */
            boolean invertRightDirection{};       /**< Reverses right command sign. */
            float64 steeringMinimum{-30.0};       /**< Minimum accepted steering angle. */
            float64 steeringMaximum{30.0};        /**< Maximum accepted steering angle. */
            float64 servoCentre{0.0};             /**< Logical servo centre in degrees. */
            float64 servoTravel{30.0};            /**< Allowed travel on each side of centre. */
            float64 initialBatteryVoltage{8.2};   /**< Initial logical battery voltage. */
            float64 batteryWarningVoltage{7.2};   /**< Warning threshold above critical. */
            float64 batteryCriticalVoltage{6.6};  /**< Critical safe-stop threshold. */
            float64 batteryReductionPerTick{0.0}; /**< Deterministic voltage reduction. */
            uint64 cameraDelayTicks{};            /**< Logical delay added for each camera sample. */
            boolean freezeCamera{};               /**< Repeats the same camera-frame identifier. */
            uint32 i2cFailureInterval{};          /**< Zero disables; otherwise every Nth operation fails. */
            std::vector<std::array<uint16, 3U>> grayscaleSequence{}; /**< Bounded samples. */
            std::vector<float64> ultrasonicSequence{};               /**< Bounded centimeter samples. */
    };

    /** @brief Stores deterministic state exposed directly for assertions. */
    struct XWalkLogicalModelState
    {
            XWalkLogicalModelConfiguration configuration{};
            float64 commandedLeftSpeed{};
            float64 commandedRightSpeed{};
            float64 simulatedLeftSpeed{};
            float64 simulatedRightSpeed{};
            float64 steeringAngle{};
            float64 batteryVoltage{};
            uint64 logicalTime{};
            uint64 nextEventSequence{1U};
            uint64 cameraFrameIdentifier{};
            uint32 i2cOperationCount{};
            size grayscaleIndex{};
            size ultrasonicIndex{};
            std::array<XWalkLogicalModelEvent, XWALK_LOGICAL_MODEL_MAXIMUM_EVENTS> events{};
            size eventCount{};
            uint64 droppedEvents{};
            boolean initialized{};
            boolean armed{};
            boolean batteryWarning{};
            boolean batteryCritical{};
    };

    /** @brief Validates every model parameter and sequence bound. */
    XWalkLogicalModelStatus
    validateLogicalModelConfiguration(const XWalkLogicalModelConfiguration& configuration) noexcept;
    /** @brief Initializes one safe, deterministic logical state. */
    XWalkLogicalModelStatus initializeLogicalModel(XWalkLogicalModelState& state,
                                                   const XWalkLogicalModelConfiguration& configuration) noexcept;
    /** @brief Applies clamped left and right motor commands. */
    XWalkLogicalModelStatus
    commandLogicalMotors(XWalkLogicalModelState& state, float64 leftSpeed, float64 rightSpeed) noexcept;
    /** @brief Applies rate limits, battery reduction and logical time advancement. */
    XWalkLogicalModelStatus advanceLogicalModel(XWalkLogicalModelState& state, uint64 ticks) noexcept;
    /** @brief Applies configured centre, travel and steering clamps. */
    XWalkLogicalModelStatus commandLogicalSteering(XWalkLogicalModelState& state, float64 angle) noexcept;
    /** @brief Returns the next bounded grayscale sample. */
    XWalkLogicalModelStatus nextLogicalGrayscale(XWalkLogicalModelState& state,
                                                 std::array<uint16, 3U>& sample) noexcept;
    /** @brief Returns the next bounded ultrasonic sample. */
    XWalkLogicalModelStatus nextLogicalUltrasonic(XWalkLogicalModelState& state, float64& distanceCentimeters) noexcept;
    /** @brief Returns one delayed or frozen logical camera frame identifier. */
    XWalkLogicalModelStatus nextLogicalCameraFrame(XWalkLogicalModelState& state, uint64& frameIdentifier) noexcept;
    /** @brief Reports whether the next configured logical I2C operation fails. */
    boolean logicalI2cOperationFails(XWalkLogicalModelState& state) noexcept;
    /** @brief Invalidates movement and disarms the model after any terminal fault. */
    void enterLogicalSafeState(XWalkLogicalModelState& state) noexcept;

} /* namespace xwalk::hal::simulation */

#endif /* XHAL_RPI5CAR_LOGICAL_MODELS_H */
