/******************************************************************************
 * @file        xControllerParsingTest.cpp
 * @brief       Verifies Controller request parsing and output formatting.
 *
 * @details
 * Exercises the device-free application boundary with valid requests,
 * boundary values, and representative invalid command text.
 *
 * @project     xWalk Firmware
 * @module      xWalkApp GoogleTest
 *
 * @author      Joxy John
 * @date        2026-08-06
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

#include "xControllerParsing.h"

#include "xHal_Rpi5CarTypes.h"

#include <gtest/gtest.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains device-free Controller parsing test cases. */
namespace
{

/******************************************************************************
 * Test function definitions
 ******************************************************************************/

/** @brief Verifies hexadecimal payload conversion in both directions. */
TEST(XWalkAppParsingGroup, HexadecimalPayload)
{
    const ctrl::bytevector bytes = xwalk::ctrl::XWALK_parseHexBytes("0x0AfE");

    ASSERT_EQ(bytes.size(), 2U);
    EXPECT_EQ(bytes[0U], 0x0AU);
    EXPECT_EQ(bytes[1U], 0xFEU);
    EXPECT_EQ(xwalk::ctrl::XWALK_formatHexBytes(bytes), "0A FE");
    EXPECT_THROW(xwalk::ctrl::XWALK_parseHexBytes("A"), ctrl::invalidargument);
    EXPECT_THROW(xwalk::ctrl::XWALK_parseHexBytes("GG"), ctrl::invalidargument);
}

/** @brief Verifies request parsers for movement, steering, camera, and sensing. */
TEST(XWalkAppParsingGroup, VehicleRequests)
{
    const xwalk::ctrl::XWalkLifecycleRequest lifecycle =
        xwalk::ctrl::XWALK_parseLifecycleRequest({"service", "stop"}, "invalid");
    const xwalk::ctrl::XWalkMoveRequest move = xwalk::ctrl::XWALK_parseMoveRequest(
        {"move", "backward", "--speed=75", "--duration", "2.5"});
    const xwalk::ctrl::XWalkMoveRequest demo =
        xwalk::ctrl::XWALK_parseMoveRequest({"move", "demo"});
    const xwalk::ctrl::XWalkTurnRequest turn =
        xwalk::ctrl::XWALK_parseTurnRequest({"turn", "right", "--angle", "20"});
    const xwalk::ctrl::XWalkCameraRequest camera =
        xwalk::ctrl::XWALK_parseCameraRequest({"cam", "tilt", "--angle", "-15"});
    const xwalk::ctrl::XWalkSensorRequest sensor =
        xwalk::ctrl::XWALK_parseSensorRequest({"sensor", "grayscale"});
    const xwalk::ctrl::XWalkSelfDriveRequest selfDrive =
        xwalk::ctrl::XWALK_parseSelfDriveRequest({"self-drive", "wave-hands"});

    EXPECT_EQ(lifecycle.action, xwalk::ctrl::XWalkLifecycleAction::Stop);
    EXPECT_EQ(move.action, xwalk::ctrl::XWalkMoveAction::Backward);
    EXPECT_DOUBLE_EQ(move.speedPercent, 75.0);
    EXPECT_EQ(move.durationMs, 2'500U);
    EXPECT_EQ(demo.action, xwalk::ctrl::XWalkMoveAction::Demo);
    EXPECT_EQ(turn.direction, xwalk::ctrl::XWalkTurnDirection::Right);
    EXPECT_DOUBLE_EQ(turn.angleDegrees, 20.0);
    EXPECT_EQ(camera.axis, xwalk::ctrl::XWalkCameraAxis::Tilt);
    EXPECT_DOUBLE_EQ(camera.angleDegrees, -15.0);
    EXPECT_EQ(sensor.type, xwalk::ctrl::XWalkSensorType::Grayscale);
    EXPECT_EQ(selfDrive.action, "wave hands");
    EXPECT_NO_THROW(xwalk::ctrl::XWALK_parseNoArgumentRequest({"doctor"}, "invalid"));
    EXPECT_THROW(xwalk::ctrl::XWALK_parseNoArgumentRequest(
        {"doctor", "extra"}, "invalid"), ctrl::invalidargument);
}

/** @brief Verifies media, SPI, GPT-car, and calibration request parsing. */
TEST(XWalkAppParsingGroup, SpecializedRequests)
{
    const xwalk::ctrl::XWalkSoundRequest play = xwalk::ctrl::XWALK_parseSoundRequest(
        {"sound", "play", "horn.wav", "--volume", "80"});
    const xwalk::ctrl::XWalkSoundRequest music =
        xwalk::ctrl::XWALK_parseSoundRequest({"sound", "music", "theme.mp3"});
    const xwalk::ctrl::XWalkSoundRequest volume =
        xwalk::ctrl::XWALK_parseSoundRequest({"sound", "volume", "60"});
    const xwalk::ctrl::XWalkSpiRequest spi =
        xwalk::ctrl::XWALK_parseSpiRequest({"spi", "transfer", "12AB"});
    const xwalk::ctrl::XWalkGptCarRequest gpt = xwalk::ctrl::XWALK_parseGptCarRequest(
        {"gpt-car", "start", "--keyboard", "--no-img"});

    EXPECT_EQ(play.operation, xwalk::ctrl::XWalkSoundOperation::Play);
    ASSERT_TRUE(play.volumePercent.has_value());
    EXPECT_DOUBLE_EQ(*play.volumePercent, 80.0);
    ASSERT_TRUE(music.volumePercent.has_value());
    EXPECT_DOUBLE_EQ(*music.volumePercent, 20.0);
    EXPECT_EQ(volume.operation, xwalk::ctrl::XWalkSoundOperation::Volume);
    ASSERT_EQ(spi.transmitData.size(), 2U);
    EXPECT_EQ(spi.transmitData[1U], 0xABU);
    EXPECT_TRUE(gpt.keyboardInput);
    EXPECT_FALSE(gpt.withImage);
    EXPECT_EQ(xwalk::ctrl::XWALK_parseCalibrationRequest({"calibrate"}).mode,
        xwalk::ctrl::XWalkCalibrationMode::Complete);
    EXPECT_EQ(xwalk::ctrl::XWALK_parseCalibrationRequest(
        {"calibrate", "grayscale"}).mode,
        xwalk::ctrl::XWalkCalibrationMode::Grayscale);
    EXPECT_EQ(xwalk::ctrl::XWALK_parseCalibrationRequest(
        {"calibrate", "servo-motor"}).mode,
        xwalk::ctrl::XWalkCalibrationMode::ServoMotor);
}

/** @brief Verifies shared option and numeric conversion validation. */
TEST(XWalkAppParsingGroup, OptionsAndNumbers)
{
    const xwalk::ctrl::controlleroptions options = xwalk::ctrl::XWALK_parseOptions(
        {"move", "forward", "--speed=40", "--duration", "2.5"}, 2U);

    EXPECT_EQ(xwalk::ctrl::XWALK_optionValue(options, "speed", "50", false), "40");
    EXPECT_EQ(xwalk::ctrl::XWALK_optionValue(options, "missing", "default", false),
        "default");
    EXPECT_NO_THROW(xwalk::ctrl::XWALK_validateOptions(
        options, {"speed", "duration"}));
    EXPECT_THROW(xwalk::ctrl::XWALK_validateOptions(options, {"speed"}),
        ctrl::invalidargument);
    EXPECT_THROW(xwalk::ctrl::XWALK_optionValue(
        options, "missing", "", true), ctrl::invalidargument);
    EXPECT_DOUBLE_EQ(xwalk::ctrl::XWALK_parseNumber("100", "value", 0.0, 100.0),
        100.0);
    EXPECT_THROW(xwalk::ctrl::XWALK_parseNumber("value", "value", 0.0, 100.0),
        ctrl::invalidargument);
    EXPECT_THROW(xwalk::ctrl::XWALK_parseNumber("101", "value", 0.0, 100.0),
        ctrl::outofrange);
    EXPECT_EQ(xwalk::ctrl::XWALK_durationMilliseconds(2.5), 2'500U);
}

/** @brief Verifies stable formatting used by Controller output handlers. */
TEST(XWalkAppParsingGroup, ResultFormatting)
{
    const xwalk::hal::linetrackervalues values{1, 2, 3};
    const xwalk::hal::linetrackerstatus status{1U, 0U, 1U};
    const xwalk::agent::XWalkComputerVisionDetection detection{1U, 10, 20, 30U, 40U};

    EXPECT_EQ(xwalk::ctrl::XWALK_formatOneDecimal(12.34), "12.3");
    EXPECT_EQ(xwalk::ctrl::XWALK_formatValues(values), "[1, 2, 3]");
    EXPECT_EQ(xwalk::ctrl::XWALK_formatStatus(status), "[1, 0, 1]");
    EXPECT_EQ(xwalk::ctrl::XWALK_formatLineTrackingState(
        xwalk::agent::XWalkLineTrackingState::Stop), "stop");
    EXPECT_EQ(xwalk::ctrl::XWALK_formatLineTrackingState(
        xwalk::agent::XWalkLineTrackingState::Forward), "forward");
    EXPECT_EQ(xwalk::ctrl::XWALK_formatLineTrackingState(
        xwalk::agent::XWalkLineTrackingState::Left), "left");
    EXPECT_EQ(xwalk::ctrl::XWALK_formatLineTrackingState(
        xwalk::agent::XWalkLineTrackingState::Right), "right");
    EXPECT_EQ(xwalk::ctrl::XWALK_formatDetection(detection),
        "Coordinate:(10, 20) Size (30, 40)");
}

} /* namespace */
