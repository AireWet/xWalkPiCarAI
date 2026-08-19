/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxLifecycle.cpp
 * @brief       Implements PiCar-X coordinator lifecycle and configuration
 *loading.
 *
 * @details
 * Binds caller-owned HAL dependencies, parses persisted calibration, and
 *initializes safe actuator positions.
 *
 * @project     xWalk Firmware
 * @module      xWalkPicarx
 *
 * @author      Joxy John
 * @date        2026-07-31
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

#include "xAgent_Rpi5CarPicarx.h"

#include "xHal_Rpi5CarTrace.h"

#include <memory>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains application coordinators for the
 * xWalk firmware. */
namespace xwalk::agent
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs a PiCar-X coordinator and loads persisted calibration.
     *
     * @param[in] motors Paired motors that must outlive this object.
     * @param[in] directionServo Steering servo that must outlive this object.
     * @param[in] cameraPanServo Camera pan servo that must outlive this object.
     * @param[in] cameraTiltServo Camera tilt servo that must outlive this object.
     * @param[in] grayscale Grayscale module that must outlive this object.
     * @param[in] ultrasonic Ultrasonic sensor that must outlive this object.
     * @param[in] configStore Configuration store that must outlive this object.
     */
    XWalkPicarx::XWalkPicarx(hal::XWalkMotors& motors,
                             hal::XWalkServo& directionServo,
                             hal::XWalkServo& cameraPanServo,
                             hal::XWalkServo& cameraTiltServo,
                             hal::XWalkGrayscaleModule& grayscale,
                             hal::XWalkUltrasonic& ultrasonic,
                             hal::XWalkConfigStore& configStore)
        : motorsObject(&motors), directionServoObject(&directionServo), cameraPanServoObject(&cameraPanServo),
          cameraTiltServoObject(&cameraTiltServo), grayscaleObject(&grayscale), ultrasonicObject(&ultrasonic),
          configStoreObject(&configStore)
    {
        loadConfiguration();
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Latches an emergency stop without releasing any non-owning dependency.
     */
    XWalkPicarx::~XWalkPicarx() noexcept
    {
        static_cast<void>(emergencyStop());
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /** @brief Loads and validates persisted calibration without commanding
     * actuators. */
    void XWalkPicarx::loadConfiguration()
    {
        directionCalibrationDegreesValue =
            parseFloat(configStoreObject->get("picarx_dir_servo", "0"), "direction calibration");
        cameraPanCalibrationDegreesValue =
            parseFloat(configStoreObject->get("picarx_cam_pan_servo", "0"), "camera pan calibration");
        cameraTiltCalibrationDegreesValue =
            parseFloat(configStoreObject->get("picarx_cam_tilt_servo", "0"), "camera tilt calibration");
        configuredMaximumMotorOutputPercentValue =
            parseFloat(configStoreObject->get("picarx_max_motor_output_percent", "20"), "maximum motor output");
        if ((configuredMaximumMotorOutputPercentValue < 0.0) || (configuredMaximumMotorOutputPercentValue > 100.0))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "maximum motor output must be between 0 and 100 percent");
        }
        const agent::string calibrationVerifiedText = configStoreObject->get("picarx_calibration_verified", "false");
        if (calibrationVerifiedText == "true")
        {
            calibrationVerifiedValue = true;
        }
        else if (calibrationVerifiedText != "false")
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "picarx calibration verification must be true or false");
        }
        const agent::string applyPersistedServoPositionsText =
            configStoreObject->get("picarx_apply_persisted_servo_positions", "false");
        if (applyPersistedServoPositionsText == "true")
        {
            applyPersistedServoPositionsValue = true;
        }
        else if (applyPersistedServoPositionsText != "false")
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "persisted servo position gate must be true or false");
        }
        maximumMotorOutputPercentValue =
            calibrationVerifiedValue
                ? configuredMaximumMotorOutputPercentValue
                : ((configuredMaximumMotorOutputPercentValue < 20.0) ? configuredMaximumMotorOutputPercentValue : 20.0);

        const agent::float64 motorSpeedCalibration =
            parseFloat(configStoreObject->get("picarx_motor_speed_calibration", "0"), "motor speed calibration");
        if ((motorSpeedCalibration < -100.0) || (motorSpeedCalibration > 100.0))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE,
                                 "motor speed calibration must be between "
                                 "-100 and 100 percentage points");
        }
        motorSpeedCalibrationValues = {};
        if (motorSpeedCalibration < 0.0)
        {
            motorSpeedCalibrationValues[1U] = -motorSpeedCalibration;
        }
        else
        {
            motorSpeedCalibrationValues[0U] = motorSpeedCalibration;
        }

        const agent::fixedarray<agent::int32, 2U> motorDirections =
            parseMotorDirections(configStoreObject->get("picarx_dir_motor", "[1, 1]"));
        for (agent::uint32 index = 0U; index < 2U; ++index)
        {
            if ((motorDirections[index] != 1) && (motorDirections[index] != -1))
            {
                XWALK_RPIAGENT_ERROR(XWALK_INVAL, "motor directions must contain only 1 or -1");
            }
        }
        motorsObject->setLeftReversed(motorDirections[0U] < 0);
        motorsObject->setRightReversed(motorDirections[1U] < 0);

        const hal::linetrackervalues lineReferences =
            parseReferences(configStoreObject->get("line_reference", "[1000, 1000, 1000]"), "line references");
        cliffReferenceValues =
            parseReferences(configStoreObject->get("cliff_reference", "[500, 500, 500]"), "cliff references");
        grayscaleObject->setReference(lineReferences);
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    agent::boolean XWalkPicarx::initialize()
    {
        if (initializedValue)
        {
            return false;
        }

        const auto rollbackInitialization = [this](void*) noexcept
        {
            static_cast<void>(emergencyStop());
            initializedValue = false;
        };
        std::unique_ptr<void, decltype(rollbackInitialization)> rollbackGuard(this, rollbackInitialization);
        static_cast<void>(directionServoObject->initialize());
        static_cast<void>(cameraPanServoObject->initialize());
        static_cast<void>(cameraTiltServoObject->initialize());
        if (applyPersistedServoPositionsValue)
        {
            directionServoObject->setAngle(directionCalibrationDegreesValue);
            cameraPanServoObject->setAngle(cameraPanCalibrationDegreesValue);
            cameraTiltServoObject->setAngle(cameraTiltCalibrationDegreesValue);
        }
        motorsObject->arm();
        emergencyStopRequestedValue.store(false);
        initializedValue = true;
        static_cast<void>(rollbackGuard.release()); // NOLINT(bugprone-unused-return-value): disarms rollback only.
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .001, "PiCar-X coordinator initialized and motors armed at zero output");
        return true;
    }

    agent::boolean XWalkPicarx::isInitialized() const noexcept
    {
        return initializedValue;
    }

} /* namespace xwalk::agent */
