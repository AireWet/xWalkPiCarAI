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
#include <cassert>
namespace {
using namespace xwalk::hal;
using namespace xwalk::hal::test::boardcontrol;
void testSupportedDevice(const filesystempath &rootPath) {
  const filesystempath nodePath = rootPath / "hat-v5";
  createSupportedNode(nodePath);
  XWalkDevice device(rootPath.string());
  const XWalkDeviceInformation &info = device.information();
  assert(info.detected);
  assert(info.model == XWalkDeviceModel::RobotHatV5);
  assert(info.productName == "Robot HAT 5");
  assert(info.productId == 0x1902U);
  assert(info.productVersion == 0x50U);
  assert(info.uuid == XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID);
  assert(info.vendor == "SunFounder");
  assert(info.speakerEnablePin == 12U);
  assert(info.motorMode == 2U);
  assert(device.deviceTreeRoot() == rootPath);
  writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_UUID_PROPERTY,
                "unsupported-uuid", true);
  device.refresh();
  assert(!device.information().detected);
  assert(device.information().model == XWalkDeviceModel::RobotHatV4);
  assert(device.information().speakerEnablePin == 20U);
  assert(device.information().motorMode == 1U);
  removeNode(nodePath);
}
void testCandidateFiltering(const filesystempath &rootPath) {
  const filesystempath nodePath = rootPath / "board-v5";
  createSupportedNode(nodePath);
  XWalkDevice device(rootPath.string());
  assert(!device.information().detected);
  removeNode(nodePath);
}
void testPropertyValidation(const filesystempath &rootPath) {
  const filesystempath nodePath = rootPath / "hat-invalid";
  createSupportedNode(nodePath);
  static_cast<void>(
      removeFilesystemEntry(nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY));
  xwalk::hal::test::expectFailure(
      [&]() { XWalkDevice device(rootPath.string()); });
  writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY, "SunFounder",
                false);
  writeProperty(nodePath / XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY, "xyz",
                true);
  xwalk::hal::test::expectFailure(
      [&]() { XWalkDevice device(rootPath.string()); });
  removeNode(nodePath);
}
void testRootValidation(const filesystempath &rootPath) {
  XWalkDevice device(rootPath.string());
  assert(!device.information().detected);
  assert(device.information().model == XWalkDeviceModel::RobotHatV4);
  xwalk::hal::test::expectFailure([&]() { XWalkDevice invalidDevice(""); });
}
} /* namespace */
XWalkHal::int32 main(XWalkHal::int32 argumentCount,
                     XWalkHal::charpointer arguments[]) {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH);
  assert(argumentCount == 2);
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
