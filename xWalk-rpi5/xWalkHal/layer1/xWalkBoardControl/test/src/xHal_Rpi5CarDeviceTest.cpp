/******************************************************************************
 * @file        xHal_Rpi5CarDeviceTest.cpp
 * @brief       Verifies device discovery through named filesystem fixtures.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Test
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBoardControlTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
namespace
{
    using namespace xwalk::hal;
    using namespace xwalk::hal::test::boardcontrol;
    void testSupportedDevice(const filesystempath& rootPath)
    {
        const filesystempath nodePath = rootPath / "hat-v5";
        createSupportedNode(nodePath);
        XWalkDevice device(rootPath.string());
        const XWalkDeviceInformation& info = device.information();
        xwalk::hal::test::requireTestCondition(info.detected);
        xwalk::hal::test::requireTestCondition(info.model == XWalkDeviceModel::RobotHatV5);
        xwalk::hal::test::requireTestCondition(info.productName == "Robot HAT 5");
        xwalk::hal::test::requireTestCondition(info.productId == 0x1902U);
        xwalk::hal::test::requireTestCondition(info.productVersion == 0x50U);
        xwalk::hal::test::requireTestCondition(info.uuid == XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID);
        xwalk::hal::test::requireTestCondition(info.vendor == "SunFounder");
        xwalk::hal::test::requireTestCondition(info.speakerEnablePin == 12U);
        xwalk::hal::test::requireTestCondition(info.motorMode == 2U);
        xwalk::hal::test::requireTestCondition(device.deviceTreeRoot() == rootPath);
        writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY, "unsupported-uuid", true);
        device.refresh();
        xwalk::hal::test::requireTestCondition(!device.information().detected);
        xwalk::hal::test::requireTestCondition(device.information().model == XWalkDeviceModel::RobotHatV4);
        xwalk::hal::test::requireTestCondition(device.information().speakerEnablePin == 20U);
        xwalk::hal::test::requireTestCondition(device.information().motorMode == 1U);
        removeNode(nodePath);
    }
    void testCandidateFiltering(const filesystempath& rootPath)
    {
        const filesystempath nodePath = rootPath / "board-v5";
        createSupportedNode(nodePath);
        XWalkDevice device(rootPath.string());
        xwalk::hal::test::requireTestCondition(!device.information().detected);
        removeNode(nodePath);
    }
    void testPropertyValidation(const filesystempath& rootPath)
    {
        const filesystempath nodePath = rootPath / "hat-invalid";
        createSupportedNode(nodePath);
        static_cast<void>(removeFilesystemEntry(nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY));
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkDevice device(rootPath.string());
            });
        writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY, "SunFounder", false);
        writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY, "xyz", true);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkDevice device(rootPath.string());
            });
        removeNode(nodePath);
    }
    void testRootValidation(const filesystempath& rootPath)
    {
        XWalkDevice device(rootPath.string());
        xwalk::hal::test::requireTestCondition(!device.information().detected);
        xwalk::hal::test::requireTestCondition(device.information().model == XWalkDeviceModel::RobotHatV4);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkDevice invalidDevice("");
            });
    }
} /* namespace */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH);
    xwalk::hal::test::requireTestCondition(argumentCount == 2);
    const XWalkHal::filesystempath rootPath(arguments[1]);
    static_cast<void>(XWalkHal::createDirectories(rootPath));
    XWALK_HAL_TRACE_UID0(RPI .335, "xWalkDevice host tests started");
    testSupportedDevice(rootPath);
    testCandidateFiltering(rootPath);
    testPropertyValidation(rootPath);
    testRootValidation(rootPath);
    XWALK_HAL_TRACE_UID0(RPI .336, "xWalkDevice host tests completed");
    static_cast<void>(XWalkHal::removeFilesystemEntry(rootPath));
    return 0;
}
