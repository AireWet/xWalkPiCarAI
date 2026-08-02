/******************************************************************************
 * @file        xHal_Rpi5CarRobotLifecycle.cpp
 * @brief       Implements xWalk robot construction, registration, and initialization.
 *
 * @details
 * Binds caller-owned dependencies, validates bounded servo registration, parses
 * persisted offsets, and initializes servos in a caller-selected order.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarRobot.h"

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
 * @brief Returns a registered non-null servo at a validated zero-based index.
 *
 * @param[in] index
 * Zero-based servo index.
 *
 * @return
 * Registered servo reference.
 *
 * @throws outofrange
 * If `index` does not identify a registered servo.
 */
XWalkServo& XWalkRobot::servoAt(uint32 index) const
{
    if (index >= servoCountValue)
    {
        XHAL_THROW_OUT_OF_RANGE("Robot servo index is outside its range");
    }
    return *servoObjects[index];
}

/**
 * @brief Returns one value repeated for every registered servo.
 *
 * @param[in] defaultValue
 * Value copied into each vector element.
 *
 * @return
 * Vector containing `servoCountValue` elements.
 */
float64vector XWalkRobot::newList(float64 defaultValue) const
{
    return float64vector(servoCountValue, defaultValue);
}

/**
 * @brief Validates that a vector has one finite value per registered servo.
 *
 * @param[in] angles
 * Values to validate.
 *
 * @param[in] description
 * Non-empty description used in the exception message.
 *
 * @throws invalidargument
 * If the vector length differs from the servo count or any value is non-finite.
 */
void XWalkRobot::validateAngles(const float64vector& angles, stringview description) const
{
    if (angles.size() != servoCountValue)
    {
        XHAL_THROW_INVALID_ARGUMENT_DETAIL(description, " count must match registered servos");
    }
    for (const float64 angle : angles)
    {
        if (!XHAL_IS_FINITE(angle))
        {
            XHAL_THROW_INVALID_ARGUMENT_DETAIL(description, " values must be finite");
        }
    }
}

/**
 * @brief Parses one persisted bracketed comma-separated offset list.
 *
 * @param[in] serializedOffsets
 * Text in the form `[value,value]` with one value per registered servo.
 *
 * @return
 * Parsed finite offsets in degrees.
 *
 * @throws invalidargument
 * If the text is malformed, contains a non-finite value, or has the wrong count.
 *
 * @throws outofrange
 * If a numeric value cannot be represented as `float64`.
 */
float64vector XWalkRobot::parseOffsets(stringview serializedOffsets) const
{
    if ((serializedOffsets.size() < 2U) || (serializedOffsets.front() != '[') ||
        (serializedOffsets.back() != ']'))
    {
        XHAL_THROW_INVALID_ARGUMENT("Robot offset configuration is malformed");
    }

    const string content(serializedOffsets.substr(1U, serializedOffsets.size() - 2U));
    float64vector parsedOffsets;
    size tokenStart = 0U;
    while (tokenStart <= content.size())
    {
        const size separator = content.find(',', tokenStart);
        const size tokenLength = (separator == string::npos) ? string::npos : separator - tokenStart;
        const string token = content.substr(tokenStart, tokenLength);
        if (token.empty())
        {
            XHAL_THROW_INVALID_ARGUMENT("Robot offset configuration contains an empty value");
        }

        size parsedLength = 0U;
        const float64 parsedValue = common::parseFloat64(token, parsedLength);
        if ((parsedLength != token.size()) || !XHAL_IS_FINITE(parsedValue))
        {
            XHAL_THROW_INVALID_ARGUMENT("Robot offset configuration contains an invalid value");
        }
        parsedOffsets.push_back(parsedValue);

        if (separator == string::npos)
        {
            break;
        }
        tokenStart = separator + 1U;
    }
    validateAngles(parsedOffsets, "Robot offset");
    return parsedOffsets;
}

/**
 * @brief Serializes one offset vector as a bracketed comma-separated list.
 *
 * @param[in] offsets
 * Finite offset values in degrees.
 *
 * @return
 * Configuration-store representation.
 */
string XWalkRobot::serializeOffsets(const float64vector& offsets)
{
    string serialized("[");
    for (size index = 0U; index < offsets.size(); ++index)
    {
        if (index != 0U)
        {
            serialized.push_back(',');
        }
        serialized.append(common::float64ToString(offsets[index]));
    }
    serialized.push_back(']');
    return serialized;
}

/**
 * @brief Requires successful initialization before positional operations.
 *
 * @throws runtimeerror
 * If `initialize()` has not completed.
 */
void XWalkRobot::requireInitialized() const
{
    if (!initializedValue)
    {
        XHAL_THROW_RUNTIME_ERROR("Robot must be initialized before use");
    }
}

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

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
XWalkRobot::XWalkRobot(XWalkConfigStore& store, stringview name, uint32 initializationDelayMs):
    configStore(&store), nameValue(name), offsetKeyValue(string(name) + "_servo_offset_list"),
    initializationDelayMsValue(initializationDelayMs)
{
    if (name.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("Robot name must not be empty");
    }
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the robot without releasing its non-owning dependencies.
 */
XWalkRobot::~XWalkRobot() = default;

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

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
void XWalkRobot::addServo(XWalkServo& servo, float64 initialAngleDegrees)
{
    if (initializedValue || !XHAL_IS_FINITE(initialAngleDegrees))
    {
        XHAL_THROW_INVALID_ARGUMENT("Robot servo registration is invalid");
    }
    if (servoCountValue >= XHAL_RPI5CAR_ROBOT_MAX_SERVOS)
    {
        XHAL_THROW_OUT_OF_RANGE("Robot supports at most 12 servos");
    }

    servoObjects[servoCountValue] = &servo;
    servoPositionsValue.push_back(initialAngleDegrees);
    originPositionsValue.push_back(0.0);
    calibrationPositionsValue.push_back(0.0);
    directionValues.push_back(1.0);
    offsetValues.push_back(0.0);
    ++servoCountValue;
}

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
void XWalkRobot::initialize(const uint32vector& initializationOrder)
{
    if ((servoCountValue == 0U) || initializedValue)
    {
        XHAL_THROW_INVALID_ARGUMENT("Robot initialization state is invalid");
    }

    uint32vector order = initializationOrder;
    if (order.empty())
    {
        for (uint32 index = 0U; index < servoCountValue; ++index)
        {
            order.push_back(index);
        }
    }
    for (const uint32 index : order)
    {
        static_cast<void>(servoAt(index));
    }

    const string defaults = serializeOffsets(newList(0.0));
    offsetValues = parseOffsets(configStore->get(offsetKeyValue, defaults));
    for (const uint32 index : order)
    {
        servoAt(index).setAngle(offsetValues[index] + servoPositionsValue[index]);
        common::sleepMilliseconds(initializationDelayMsValue);
    }
    initializedValue = true;
}

} /* namespace xwalk::hal */
