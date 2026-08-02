/******************************************************************************
 * @file        xHal_Rpi5CarDeviceTest.cpp
 * @brief       Verifies device discovery using synthetic device-tree files.
 *
 * @details
 * Checks v5 UUID recognition, metadata parsing, legacy defaults, candidate
 * filtering, refresh behavior, and malformed or incomplete property rejection.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Test
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

#include "xHal_Rpi5CarDevice.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <filesystem>
#include <fstream>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test filesystem helpers private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Writes one synthetic device-tree property.
 *
 * @param[in] path
 * Property file path whose parent already exists.
 *
 * @param[in] value
 * Textual property payload.
 *
 * @param[in] appendNull
 * `true` to append one device-tree terminal null byte.
 */
void writeProperty(const XWalkHal::filesystempath& path, XWalkHal::stringview value,
    XWalkHal::boolean appendNull)
{
    XWalkHal::outputfilestream file(path, XWalkHal::FILE_OPEN_WRITE_TRUNCATE);
    assert(file.is_open());
    file << XWalkHal::string(value);
    if (appendNull)
    {
        file.put('\0');
    }
    file.close();
    assert(!file.fail());
}

/**
 * @brief Removes every known synthetic property and its containing node.
 *
 * @param[in] nodePath
 * Exact test node path created by this test executable.
 */
void removeNode(const XWalkHal::filesystempath& nodePath)
{
    static_cast<void>(XWalkHal::removeFilesystemEntry(
        nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_PROPERTY));
    static_cast<void>(XWalkHal::removeFilesystemEntry(
        nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY));
    static_cast<void>(XWalkHal::removeFilesystemEntry(
        nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_VERSION_PROPERTY));
    static_cast<void>(XWalkHal::removeFilesystemEntry(
        nodePath / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY));
    static_cast<void>(XWalkHal::removeFilesystemEntry(
        nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY));
    static_cast<void>(XWalkHal::removeFilesystemEntry(nodePath));
}

/**
 * @brief Creates one complete supported Robot HAT v5 node.
 *
 * @param[in] nodePath
 * Node path to create below the synthetic root.
 */
void createSupportedNode(const XWalkHal::filesystempath& nodePath)
{
    static_cast<void>(XWalkHal::createDirectories(nodePath));
    writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_PROPERTY,
        "Robot HAT 5", false);
    writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY,
        "00001902", true);
    writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_VERSION_PROPERTY,
        "0x00000050", true);
    writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY,
        XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID, true);
    writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY,
        "SunFounder", false);
}

/** @brief Verifies complete v5 metadata and board configuration discovery. */
void testSupportedDevice(const XWalkHal::filesystempath& rootPath)
{
    const XWalkHal::filesystempath nodePath = rootPath / "hat-v5";
    createSupportedNode(nodePath);
    XWalkHal::XWalkDevice device(rootPath.string());
    const XWalkHal::XWalkDeviceInformation& information = device.information();
    assert(information.detected);
    assert(information.model == XWalkHal::XWalkDeviceModel::RobotHatV5);
    assert(information.productName == "Robot HAT 5");
    assert(information.productId == 0x1902U);
    assert(information.productVersion == 0x50U);
    assert(information.uuid == XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID);
    assert(information.vendor == "SunFounder");
    assert(information.speakerEnablePin == 12U);
    assert(information.motorMode == 2U);
    assert(device.deviceTreeRoot() == rootPath);

    writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY,
        "unsupported-uuid", true);
    device.refresh();
    assert(!device.information().detected);
    assert(device.information().model == XWalkHal::XWalkDeviceModel::RobotHatV4);
    assert(device.information().speakerEnablePin == 20U);
    assert(device.information().motorMode == 1U);
    removeNode(nodePath);
}

/** @brief Verifies that names without the HAT marker are ignored. */
void testCandidateFiltering(const XWalkHal::filesystempath& rootPath)
{
    const XWalkHal::filesystempath ignoredPath = rootPath / "board-v5";
    createSupportedNode(ignoredPath);
    XWalkHal::XWalkDevice device(rootPath.string());
    assert(!device.information().detected);
    removeNode(ignoredPath);
}

/** @brief Verifies incomplete and malformed selected properties are rejected. */
void testPropertyValidation(const XWalkHal::filesystempath& rootPath)
{
    const XWalkHal::filesystempath nodePath = rootPath / "hat-invalid";
    createSupportedNode(nodePath);
    static_cast<void>(XWalkHal::removeFilesystemEntry(
        nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY));
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkHal::XWalkDevice device(rootPath.string());
    });

    writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY, "SunFounder", false);
    writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY, "xyz", true);
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkHal::XWalkDevice device(rootPath.string());
    });
    removeNode(nodePath);
}

/** @brief Verifies empty-root rejection and legacy defaults for an empty directory. */
void testRootValidation(const XWalkHal::filesystempath& rootPath)
{
    XWalkHal::XWalkDevice device(rootPath.string());
    assert(!device.information().detected);
    assert(device.information().model == XWalkHal::XWalkDeviceModel::RobotHatV4);

    xwalk::hal::test::expectFailure([&]()
    {
        XWalkHal::XWalkDevice invalidDevice("");
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side device-discovery tests.
 *
 * @param[in] argumentCount
 * Number of command-line arguments; exactly two are required.
 *
 * @param[in] arguments
 * Executable path followed by the module-local synthetic device-tree root.
 *
 * @return
 * Zero after every assertion passes.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    assert(argumentCount == 2);
    const XWalkHal::filesystempath rootPath(arguments[1]);
    static_cast<void>(XWalkHal::createDirectories(rootPath));
    testSupportedDevice(rootPath);
    testCandidateFiltering(rootPath);
    testPropertyValidation(rootPath);
    testRootValidation(rootPath);
    static_cast<void>(XWalkHal::removeFilesystemEntry(rootPath));
    return 0;
}
