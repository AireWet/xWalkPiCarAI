/******************************************************************************
 * @file        xAgent_Rpi5CarPicarx.h
 * @brief       Declares the PiCar-X agent coordinator.
 *
 * @details
 * Coordinates caller-owned drive, servo, sensing, and configuration HAL objects.
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

#ifndef XAGENT_RPI5CAR_PICARX_H
#define XAGENT_RPI5CAR_PICARX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarGrayscaleModule.h"
#include "xHal_Rpi5CarMotors.h"
#include "xHal_Rpi5CarServo.h"
#include "xHal_Rpi5CarUltrasonic.h"
#include "xWalk_Rpi5CarAgentConfigType.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkPicarx
     * @brief Coordinates the complete PiCar-X movement and sensing interface.
     *
     * @details
     * Stores non-owning pointers to caller-created HAL objects. Every dependency and the configuration store must
     * outlive this coordinator. Calls require external serialization when the object is shared between tasks.
     */
    class XWalkPicarx
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning paired-motor pointer that is never null after construction. */
            hal::XWalkMotors* motorsObject{nullptr};
            /** @brief Non-owning steering-servo pointer that is never null after construction. */
            hal::XWalkServo* directionServoObject{nullptr};
            /** @brief Non-owning camera-pan-servo pointer that is never null after construction. */
            hal::XWalkServo* cameraPanServoObject{nullptr};
            /** @brief Non-owning camera-tilt-servo pointer that is never null after construction. */
            hal::XWalkServo* cameraTiltServoObject{nullptr};
            /** @brief Non-owning grayscale-module pointer that is never null after construction. */
            hal::XWalkGrayscaleModule* grayscaleObject{nullptr};
            /** @brief Non-owning ultrasonic-sensor pointer that is never null after construction. */
            hal::XWalkUltrasonic* ultrasonicObject{nullptr};
            /** @brief Non-owning configuration-store pointer that is never null after construction. */
            hal::XWalkConfigStore* configStoreObject{nullptr};

            /** @brief Current constrained steering command in degrees. */
            agent::float64 directionAngleDegreesValue{};
            /** @brief Persisted steering-servo calibration offset in degrees. */
            agent::float64 directionCalibrationDegreesValue{};
            /** @brief Persisted camera-pan-servo calibration offset in degrees. */
            agent::float64 cameraPanCalibrationDegreesValue{};
            /** @brief Persisted camera-tilt-servo calibration offset in degrees. */
            agent::float64 cameraTiltCalibrationDegreesValue{};
            /** @brief Left and right motor speed corrections in percentage points. */
            agent::fixedarray<agent::float64, 2U> motorSpeedCalibrationValues{};
            /** @brief Configured motor-output limit available after first-run verification. */
            agent::float64 configuredMaximumMotorOutputPercentValue{20.0};
            /** @brief Maximum applied motor PWM magnitude in the range zero through one hundred percent. */
            agent::float64 maximumMotorOutputPercentValue{20.0};
            /** @brief Records whether the first-run actuator checks were confirmed. */
            agent::boolean calibrationVerifiedValue{false};
            /** @brief Allows initialization to apply persisted servo positions after explicit commissioning. */
            agent::boolean applyPersistedServoPositionsValue{false};
            /** @brief Latches emergency cancellation and suppresses later actuator commands. */
            agent::atomicboolean emergencyStopRequestedValue{false};
            /** @brief True after explicit servo initialization and motor arming succeeds. */
            agent::boolean initializedValue{false};
            /** @brief Per-channel cliff thresholds in raw ADC counts. */
            hal::linetrackervalues cliffReferenceValues{500, 500, 500};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Restricts one finite value to an inclusive range. */
            static agent::float64 constrain(agent::float64 value, agent::float64 minimum, agent::float64 maximum);
            /** @brief Parses one complete finite floating-point configuration value. */
            static agent::float64 parseFloat(agent::stringview text, agent::cstring name);
            /** @brief Parses exactly three signed integer values enclosed in square brackets. */
            static hal::linetrackervalues parseReferences(agent::stringview text, agent::cstring name);
            /** @brief Parses exactly two motor direction values enclosed in square brackets. */
            static agent::fixedarray<agent::int32, 2U> parseMotorDirections(agent::stringview text);
            /** @brief Formats three signed integer values using the Python-compatible list form. */
            static agent::string formatReferences(const hal::linetrackervalues& values);
            /** @brief Loads and validates persisted calibration without commanding actuators. */
            void loadConfiguration();
            /** @brief Converts and applies one Python-compatible raw motor command. */
            agent::float64 calibratedMotorSpeed(agent::uint8 motorId, agent::float64 speedPercent) const;

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a PiCar-X coordinator and loads persisted calibration.
             *
             * @param[in] ctx Non-owning dependency context whose PiCar-X fields must remain valid.
             * @pre `motors`, `dirServo`, `panServo`, `tiltServo`, `grayscale`, `ultrasonic`, and `config` are non-null.
             */
            explicit XWalkPicarx(const xAgentContext& ctx);

            /**
             * @brief Latches an emergency stop without releasing any non-owning dependency.
             */
            ~XWalkPicarx() noexcept;

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            XWalkPicarx(XWalkPicarx&&) = delete;
            XWalkPicarx(const XWalkPicarx&) = delete;
            XWalkPicarx& operator=(XWalkPicarx&&) = delete;
            XWalkPicarx& operator=(const XWalkPicarx&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Explicitly initializes servos and arms motors at zero output.
             * @details Persisted servo positions are applied only when the commissioning configuration gate is true.
             *
             * @return `true` when initialization ran; `false` when it had already completed.
             */
            agent::boolean initialize();

            /** @brief Returns whether explicit actuator initialization completed. */
            agent::boolean isInitialized() const noexcept;

            /** @brief Sets one motor command using a one-based motor identifier and -100 to 100 percent speed. */
            void setMotorSpeed(agent::uint8 motorId, agent::float64 speedPercent);
            /** @brief Sets the signed motor-speed calibration from -100 to 100 percentage points. */
            void calibrateMotorSpeed(agent::float64 value);
            /** @brief Persists motor direction as 1 or -1 for one one-based motor identifier. */
            void calibrateMotorDirection(agent::uint8 motorId, agent::int32 direction);
            /** @brief Applies one motor direction without persisting it. */
            void previewMotorDirection(agent::uint8 motorId, agent::int32 direction);
            /** @brief Returns the active left and right motor directions. */
            agent::fixedarray<agent::int32, 2U> motorDirections() const noexcept;
            /** @brief Persists and applies the steering-servo calibration offset in degrees. */
            void calibrateDirectionServo(agent::float64 value);
            /** @brief Applies a steering-servo calibration offset without persistence. */
            void previewDirectionServoCalibration(agent::float64 value);
            /** @brief Returns the active steering-servo calibration offset. */
            agent::float64 directionServoCalibration() const noexcept;
            /** @brief Sets the steering command, constrained to -30 through 30 degrees. */
            void setDirectionServoAngle(agent::float64 value);
            /** @brief Persists and applies the camera-pan calibration offset in degrees. */
            void calibrateCameraPanServo(agent::float64 value);
            /** @brief Applies a camera-pan calibration offset without persistence. */
            void previewCameraPanServoCalibration(agent::float64 value);
            /** @brief Returns the active camera-pan calibration offset. */
            agent::float64 cameraPanServoCalibration() const noexcept;
            /** @brief Persists and applies the camera-tilt calibration offset in degrees. */
            void calibrateCameraTiltServo(agent::float64 value);
            /** @brief Applies a camera-tilt calibration offset without persistence. */
            void previewCameraTiltServoCalibration(agent::float64 value);
            /** @brief Returns the active camera-tilt calibration offset. */
            agent::float64 cameraTiltServoCalibration() const noexcept;
            /** @brief Sets camera pan, constrained to -90 through 90 degrees. */
            void setCameraPanAngle(agent::float64 value);
            /** @brief Sets camera tilt, constrained to -35 through 65 degrees. */
            void setCameraTiltAngle(agent::float64 value);
            /** @brief Drives both sides with the same raw command. */
            void setPower(agent::float64 speedPercent);
            /** @brief Drives forward while reducing the inside wheel for steering. */
            void forward(agent::float64 speedPercent);
            /** @brief Drives backward while reducing the inside wheel for steering. */
            void backward(agent::float64 speedPercent);
            /** @brief Stops both drive motors. */
            void stop();
            /** @brief Attempts to refresh the active motor watchdog without throwing. */
            agent::boolean refreshMotorWatchdog() noexcept;
            /** @brief Latches actuator suppression and makes a non-throwing paired motor stop attempt. */
            agent::boolean emergencyStop() noexcept;
            /** @brief Clears the emergency latch before a new application-controlled operation. */
            void clearEmergencyStop();
            /** @brief Returns whether emergency actuator suppression is latched. */
            agent::boolean emergencyStopRequested() const noexcept;
            /** @brief Returns the configured maximum applied motor PWM magnitude in percent. */
            agent::float64 maximumMotorOutputPercent() const noexcept;
            /** @brief Persists whether motor direction, steering center, and motor balance checks passed. */
            void recordCalibrationVerified(agent::boolean verified);
            /** @brief Reports whether the first-run actuator checks were confirmed. */
            agent::boolean calibrationVerified() const noexcept;
            /** @brief Returns one ultrasonic distance measurement in centimeters. */
            agent::float64 distance();
            /** @brief Sets and persists all grayscale line references. */
            void setGrayscaleReference(const hal::linetrackervalues& value);
            /** @brief Returns the active grayscale line references. */
            const hal::linetrackervalues& grayscaleReference() const noexcept;
            /** @brief Returns current raw grayscale data. */
            hal::linetrackervalues grayscaleData();
            /** @brief Classifies supplied grayscale data against the line references. */
            hal::linetrackerstatus lineStatus(const hal::linetrackervalues& value) const noexcept;
            /** @brief Returns true when any supplied channel is at or below its cliff threshold. */
            agent::boolean cliffStatus(const hal::linetrackervalues& value) const noexcept;
            /** @brief Sets and persists all cliff thresholds. */
            void setCliffReference(const hal::linetrackervalues& value);
            /** @brief Returns the active cliff thresholds. */
            const hal::linetrackervalues& cliffReference() const noexcept;
            /** @brief Stops the motors and centers all logical actuator commands. */
            void reset();
            /** @brief Disarms motors and closes ultrasonic registrations without commanding servo movement. */
            void close();
            /** @brief Returns the current constrained steering command in degrees. */
            agent::float64 directionAngleDegrees() const noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_PICARX_H */
