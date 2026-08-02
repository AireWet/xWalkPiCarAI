/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerTest.cpp
 * @brief       Verifies grayscale and line-tracker behavior with simulated ADC data.
 *
 * @details
 * Checks raw sensing, threshold status, linear calibration, cliff and line
 * detection, position estimation, adaptive references, and input validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkLineTracker Host Test
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

#include "xHal_Rpi5CarGrayscaleModule.h"

#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarLineTracker.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains simulated ADC state and host-test scenarios.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Supplies one configurable ADC sample for each tracker channel. */
struct TestBus
{
    /** @brief Raw left, middle, and right ADC samples. */
    XWalkHal::fixedarray<XWalkHal::uint16, 3U> samples{1'200U, 800U, 1'000U};
    /** @brief Most recent ADC command received through the write callback. */
    XWalkHal::uint8 command{XHAL_RPI5CAR_ADC_READ_COMMAND};

    /** @brief Number of samples returned during the current test lifetime. */
    XWalkHal::uint32 readCount{0U};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Reports that every simulated I2C address is present.
 *
 * @param[in,out] context
 * Opaque test context.
 *
 * @param[in] address
 * Probed seven-bit address.
 *
 * @return
 * Always `true`.
 */
XWalkHal::boolean probe(XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

/**
 * @brief Records the selected ADC command.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBus`.
 *
 * @param[in] address
 * Seven-bit destination address.
 *
 * @param[in] reg
 * ADC read command containing the hardware channel mapping.
 *
 * @param[in] data
 * Command payload, unused by the simulated converter.
 */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data)
{
    static_cast<void>(address);
    static_cast<void>(data);
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.command = reg;
}

/**
 * @brief Returns the sample selected by the most recent ADC command.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBus`.
 *
 * @param[in] address
 * Seven-bit source address.
 *
 * @param[in] length
 * Requested byte count, expected to be two.
 *
 * @return
 * Big-endian simulated ADC sample.
 */
XWalkHal::bytevector read(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    static_cast<void>(address);
    static_cast<void>(length);
    TestBus& bus = *static_cast<TestBus*>(context);
    ++bus.readCount;
    const XWalkHal::uint8 hardwareChannel =
        static_cast<XWalkHal::uint8>(bus.command - XHAL_RPI5CAR_ADC_READ_COMMAND);
    const XWalkHal::uint8 logicalChannel =
        static_cast<XWalkHal::uint8>(XHAL_RPI5CAR_ADC_MAX_CHANNEL - hardwareChannel);
    const XWalkHal::uint16 sample = bus.samples[logicalChannel];
    const XWalkHal::uint16 highValue = static_cast<XWalkHal::uint16>(sample >> 8U);
    const XWalkHal::uint8 highByte = static_cast<XWalkHal::uint8>(highValue & 0xFFU);
    const XWalkHal::uint8 lowByte = static_cast<XWalkHal::uint8>(sample & 0xFFU);
    return {highByte, lowByte};
}

/**
 * @brief Verifies the complete grayscale and line-tracker behavior.
 */
void testLineTracking()
{
    TestBus bus;
    XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
    XWalkHal::XWalkAdc left(i2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkAdc middle(i2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkAdc right(i2c, 2U, XHAL_RPI5CAR_ADC_ADDRESS_1);

    XWalkHal::XWalkGrayscaleModule grayscale(left, middle, right);
    const XWalkHal::linetrackervalues grayscaleData = grayscale.read();
    assert(grayscaleData == XWalkHal::linetrackervalues({1'200, 800, 1'000}));
    const XWalkHal::linetrackerstatus defaultStatus = grayscale.readStatus();
    assert(defaultStatus == XWalkHal::linetrackerstatus({0U, 1U, 1U}));
    grayscale.setReference({1'100, 700, 1'001});
    assert(grayscale.reference() == XWalkHal::linetrackervalues({1'100, 700, 1'001}));
    const XWalkHal::linetrackerstatus updatedStatus = grayscale.readStatus();
    assert(updatedStatus == XWalkHal::linetrackerstatus({0U, 0U, 1U}));
    const XWalkHal::int32 middleValue = grayscale.readChannel(1U);
    assert(middleValue == 800);

    XWalkHal::XWalkLineTracker tracker(left, middle, right);
    const XWalkHal::linetrackervalues rawTrackerData = tracker.read(true);
    assert(rawTrackerData == XWalkHal::linetrackervalues({1'200, 800, 1'000}));
    assert(bus.readCount == 13U);
    const XWalkHal::XWalkLineCalibration manualCalibration{{1.0, 2.0, 0.5}, {0.0, 10.0, -10.0}};
    tracker.setCalibrationData(manualCalibration);
    assert(tracker.calibrateData({100, 200, 300}) == XWalkHal::linetrackervalues({100, 410, 140}));

    assert(tracker.isOnCliff({119, 500, 500}));
    assert(!tracker.isOnCliff({120, 500, 500}));
    tracker.setCliffThreshold(150);
    assert(tracker.cliffThreshold() == 150);
    assert(tracker.isOnCliff({149, 500, 500}));
    tracker.setCliffThreshold(120);

    assert(tracker.isOnLine({500, 800, 500}));
    assert(!tracker.isOnLine({500, 700, 600}));
    assert(!tracker.isOnLine({119, 800, 500}));
    const XWalkHal::float64 leftPosition = tracker.getLinePosition({200, 1'000, 1'000});
    const XWalkHal::float64 rightPosition = tracker.getLinePosition({1'000, 1'000, 200});
    const XWalkHal::float64 absentPosition = tracker.getLinePosition({1'000, 1'000, 1'000});
    assert(leftPosition == -0.53);
    assert(rightPosition == 0.53);
    assert(absentPosition == 0.0);

    tracker.updateLineBackgroundReference({400, 600, 800});
    tracker.updateLineReference({400, 600, 800});
    assert(XHAL_ABSOLUTE_VALUE(tracker.lineBackgroundReference() - 990.0) < 0.000001);
    assert(XHAL_ABSOLUTE_VALUE(tracker.lineReference() - 210.0) < 0.000001);

    const XWalkHal::XWalkLineCalibration derived =
        tracker.calibrate({1'000, 800, 900}, {100, 80, 90});
    assert(derived.slopes == XWalkHal::linetrackercalibrationvalues({1.0, 1.25, 1.11}));
    assert(derived.offsets == XWalkHal::linetrackercalibrationvalues({0.0, 0.0, 0.0}));
    assert(tracker.calibrationData().slopes == derived.slopes);
}

/**
 * @brief Verifies channel and calibration validation failures.
 */
void testValidation()
{
    TestBus bus;
    XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
    XWalkHal::XWalkAdc left(i2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkAdc middle(i2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkAdc right(i2c, 2U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    XWalkHal::XWalkGrayscaleModule grayscale(left, middle, right);
    XWalkHal::XWalkLineTracker tracker(left, middle, right);

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(grayscale.readChannel(3U));
    });

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(tracker.calibrate({100, 90, 80}, {100, 20, 10}));
    });

    const XWalkHal::float64 infinity = XHAL_POSITIVE_INFINITY(XWalkHal::float64);
    const XWalkHal::XWalkLineCalibration invalidCalibration{{1.0, infinity, 1.0}, {0.0, 0.0, 0.0}};
    xwalk::hal::test::expectFailure([&]()
    {
        tracker.setCalibrationData(invalidCalibration);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every xWalk line-tracker host-test scenario.
 *
 * @return
 * Zero when every assertion passes.
 */
XWalkHal::int32 main()
{
    testLineTracking();
    testValidation();
    return 0;
}
