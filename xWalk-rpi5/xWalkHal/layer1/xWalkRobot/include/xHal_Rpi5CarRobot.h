/******************************************************************************
 * @file        xHal_Rpi5CarRobot.h
 * @brief       Declares the xWalk coordinated multi-servo robot controller.
 *
 * @details
 * Defines bounded servo registration, persisted calibration offsets, relative
 * positioning, interpolated movement, and named multi-frame actions.
 *
 * @project     xWalk Firmware
 * @module      xWalkRobot
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

#ifndef XHAL_RPI5CAR_ROBOT_H
#define XHAL_RPI5CAR_ROBOT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarServo.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Maximum number of Robot HAT PWM servo channels accepted by one robot. */
#define XHAL_RPI5CAR_ROBOT_MAX_SERVOS 12U
/** @brief Default initialization delay between servo commands in milliseconds. */
#define XHAL_RPI5CAR_ROBOT_INITIALIZATION_DELAY_MS 150U
/** @brief Interpolation interval in milliseconds. */
#define XHAL_RPI5CAR_ROBOT_STEP_TIME_MS 10U
/** @brief Maximum supported servo speed in degrees per second at nominal voltage. */
#define XHAL_RPI5CAR_ROBOT_MAX_DPS 428.0
/** @brief Lowest persisted servo calibration offset in degrees. */
#define XHAL_RPI5CAR_ROBOT_MIN_OFFSET_DEG (-20.0)
/** @brief Highest persisted servo calibration offset in degrees. */
#define XHAL_RPI5CAR_ROBOT_MAX_OFFSET_DEG 20.0

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
     * @class XWalkRobot
     * @brief Coordinates caller-created servos as one articulated robot.
     *
     * @details
     * Stores non-owning pointers to a configuration store and up to 12 servos.
     * The application registers each servo by reference before initialization.
     * Relative commands combine origins, calibration offsets, and directions.
     */
    class XWalkRobot
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning pointer to persistent calibration storage; never null. */
            XWalkConfigStore* configStore{nullptr};
            /** @brief Non-owning nullable servo pointers; populated from `addServo()` references. */
            fixedarray<XWalkServo*, XHAL_RPI5CAR_ROBOT_MAX_SERVOS> servoObjects{};
            /** @brief Number of registered servos in the range 0 through 12. */
            uint32 servoCountValue{};
            /** @brief Robot name used to derive the persisted offset key. */
            string nameValue;
            /** @brief Configuration key containing the serialized servo offsets. */
            string offsetKeyValue;
            /** @brief Current logical servo positions in degrees. */
            float64vector servoPositionsValue;
            /** @brief Per-servo origin angles in degrees. */
            float64vector originPositionsValue;
            /** @brief Per-servo calibration target angles in degrees. */
            float64vector calibrationPositionsValue;
            /** @brief Per-servo direction multipliers, normally positive or negative one. */
            float64vector directionValues;
            /** @brief Persisted calibration offsets clamped to -20 through 20 degrees. */
            float64vector offsetValues;
            /** @brief Named ordered action frames expressed as logical servo angles. */
            orderedmap<string, float64vectorvector> actions;
            /** @brief Delay between initialization commands in milliseconds. */
            uint32 initializationDelayMsValue{};
            /** @brief Indicates that offsets were loaded and initialization commands completed. */
            boolean initializedValue{false};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Returns a registered non-null servo at a validated zero-based index.
             *
             * @param[in] index
             * Zero-based servo index.
             *
             * @return
             * Registered servo reference.
             */
            XWalkServo& servoAt(uint32 index) const;
            /**
             * @brief Returns `servoCountValue` copies of one numeric value.
             *
             * @param[in] defaultValue
             * Value copied into every element.
             *
             * @return
             * Vector containing one value per registered servo.
             */
            float64vector newList(float64 defaultValue) const;
            /**
             * @brief Validates that a vector has one finite value per registered servo.
             *
             * @param[in] angles
             * Values to validate.
             *
             * @param[in] description
             * Description used in an exception message.
             */
            void validateAngles(const float64vector& angles, stringview description) const;
            /**
             * @brief Parses one persisted bracketed comma-separated offset list.
             *
             * @param[in] serializedOffsets
             * Stored offset text containing one value per servo.
             *
             * @return
             * Parsed finite offset values in degrees.
             *
             * @throws invalidargument
             * If the stored representation is malformed or contains the wrong value count.
             *
             * @throws outofrange
             * If a stored numeric value cannot be represented as `float64`.
             */
            float64vector parseOffsets(stringview serializedOffsets) const;
            /**
             * @brief Serializes one offset vector as a bracketed comma-separated list.
             *
             * @param[in] offsets
             * Finite offset values in degrees.
             *
             * @return
             * Configuration-store representation.
             */
            static string serializeOffsets(const float64vector& offsets);
            /** @brief Requires successful initialization before positional operations. */
            void requireInitialized() const;

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs an empty robot using caller-owned configuration storage.
             *
             * @param[in] store
             * Configuration store that must outlive this robot.
             *
             * @param[in] name
             * Non-empty robot name used as the offset-key prefix.
             *
             * @param[in] initializationDelayMs
             * Delay between initialization commands in milliseconds.
             *
             * @throws invalidargument
             * If `name` is empty.
             */
            explicit XWalkRobot(XWalkConfigStore& store,
                                stringview name = "other",
                                uint32 initializationDelayMs = XHAL_RPI5CAR_ROBOT_INITIALIZATION_DELAY_MS);

            /** @brief Destroys the robot without releasing its non-owning dependencies. */
            ~XWalkRobot();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction to preserve dependency identities. */
            XWalkRobot(XWalkRobot&&) = delete;
            /** @brief Disables copying of dependency bindings. */
            XWalkRobot(const XWalkRobot&) = delete;
            /** @brief Disables move assignment of dependency bindings. */
            XWalkRobot& operator=(XWalkRobot&&) = delete;
            /** @brief Disables copy assignment of dependency bindings. */
            XWalkRobot& operator=(const XWalkRobot&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Registers one caller-created servo and its initial logical angle.
             *
             * @param[in] servo
             * Servo dependency that must outlive this robot.
             *
             * @param[in] initialAngleDegrees
             * Finite initial logical angle in degrees.
             *
             * @throws invalidargument
             * If the angle is non-finite or initialization already occurred.
             *
             * @throws outofrange
             * If 12 servos are already registered.
             */
            void addServo(XWalkServo& servo, float64 initialAngleDegrees = 0.0);

            /**
             * @brief Loads offsets and initializes registered servos in a selected order.
             *
             * @param[in] initializationOrder
             * Optional zero-based servo indices. An empty vector selects registration order.
             *
             * @throws invalidargument
             * If no servo is registered or initialization already occurred.
             *
             * @throws outofrange
             * If an order entry does not identify a registered servo.
             */
            void initialize(const uint32vector& initializationOrder = {});

            /**
             * @brief Writes one physical angle per servo without changing logical positions.
             *
             * @param[in] angles
             * One finite physical angle in degrees per servo.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @throws invalidargument
             * If the vector count is wrong or a value is non-finite.
             */
            void servoWriteRaw(const float64vector& angles);
            /**
             * @brief Writes logical angles after applying origin, direction, and offset values.
             *
             * @param[in] angles
             * One finite logical angle in degrees per servo.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @throws invalidargument
             * If the vector count is wrong or a value is non-finite.
             */
            void servoWriteAll(const float64vector& angles);

            /**
             * @brief Interpolates every servo from its current position to one target frame.
             *
             * @param[in] targets
             * One finite logical target angle in degrees per servo.
             *
             * @param[in] speedPercent
             * Finite speed clamped to 0.0 through 100.0 percent.
             *
             * @param[in] beatsPerMinute
             * Optional finite value greater than zero that overrides speed-based duration.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @throws invalidargument
             * If an input is non-finite, the target count is wrong, or BPM is not positive.
             *
             * @throws outofrange
             * If the calculated movement duration exceeds the supported `uint32` range.
             */
            void
            servoMove(const float64vector& targets, float64 speedPercent = 50.0, optionalfloat64 beatsPerMinute = {});

            /**
             * @brief Stores or replaces a named sequence of logical servo frames.
             *
             * @param[in] actionName
             * Non-empty action identifier.
             *
             * @param[in] motions
             * Ordered frames containing one finite angle per servo.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @throws invalidargument
             * If the action name is empty or a frame has invalid values.
             */
            void setAction(stringview actionName, const float64vectorvector& motions);
            /**
             * @brief Executes a named action for the requested repetition count.
             *
             * @param[in] actionName
             * Previously registered action identifier.
             *
             * @param[in] repetitions
             * Number of complete action executions.
             *
             * @param[in] speedPercent
             * Finite speed clamped to 0.0 through 100.0 percent.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @throws invalidargument
             * If the speed or a stored frame is invalid.
             *
             * @throws outofrange
             * If the action is unknown or its movement duration is unsupported.
             */
            void doAction(stringview actionName, uint32 repetitions = 1U, float64 speedPercent = 50.0);

            /**
             * @brief Clamps and persists one calibration offset per servo.
             *
             * @param[in] offsets
             * One finite offset in degrees per servo.
             *
             * @throws runtimeerror
             * If the robot is uninitialized or persistent storage cannot be updated.
             *
             * @throws invalidargument
             * If the vector count is wrong or a value is non-finite.
             */
            void setOffsets(const float64vector& offsets);
            /**
             * @brief Sets one origin angle in degrees per servo.
             *
             * @param[in] origins
             * One finite origin angle in degrees per servo.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @throws invalidargument
             * If the vector count is wrong or a value is non-finite.
             */
            void setOriginPositions(const float64vector& origins);
            /**
             * @brief Sets one calibration target angle in degrees per servo.
             *
             * @param[in] positions
             * One finite calibration target in degrees per servo.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @throws invalidargument
             * If the vector count is wrong or a value is non-finite.
             */
            void setCalibrationPositions(const float64vector& positions);
            /**
             * @brief Sets one finite direction multiplier per servo.
             *
             * @param[in] directions
             * One finite multiplier per servo, normally positive or negative one.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @throws invalidargument
             * If the vector count is wrong or a value is non-finite.
             */
            void setDirections(const float64vector& directions);

            /**
             * @brief Moves all servos to the configured calibration positions.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @post
             * Current logical positions equal the configured calibration frame.
             */
            void calibration();
            /**
             * @brief Resets logical positions to zero and writes the resulting angles.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @post
             * Every current logical position is zero degrees.
             */
            void reset();
            /**
             * @brief Resets logical positions to a supplied frame and writes it.
             *
             * @param[in] positions
             * One finite logical angle in degrees per servo.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             *
             * @throws invalidargument
             * If the vector count is wrong or a value is non-finite.
             *
             * @post
             * Current logical positions equal `positions`.
             */
            void reset(const float64vector& positions);
            /**
             * @brief Writes the zero frame without modifying stored logical positions.
             *
             * @throws runtimeerror
             * If the robot has not been initialized.
             */
            void softReset();

            /**
             * @brief Returns the number of registered servos.
             *
             * @return
             * Servo count in the range 0 through 12.
             */
            uint32 servoCount() const noexcept;
            /**
             * @brief Returns whether initialization completed.
             *
             * @return
             * `true` after successful initialization; otherwise `false`.
             */
            boolean initialized() const noexcept;
            /**
             * @brief Returns current logical positions in degrees.
             *
             * @return
             * Read-only reference valid until the next position mutation.
             */
            const float64vector& servoPositions() const noexcept;
            /**
             * @brief Returns persisted calibration offsets in degrees.
             *
             * @return
             * Read-only reference valid until offsets are changed.
             */
            const float64vector& offsets() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_ROBOT_H */
