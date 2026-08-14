/******************************************************************************
 * @file        xHal_Rpi5CarRobotTest.cpp
 * @brief       Verifies xWalkRobot through named in-memory test adapters.
 * @project     xWalk Firmware
 * @module      xWalkRobot Host Test
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarRobotTestSupport.h"
#include "xHal_Rpi5CarRobotSimulationArguments.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include <cassert>
namespace {
using namespace xwalk::hal;
using namespace xwalk::hal::test::robot;
void testRobot(const filesystempath &filePath) {
  TestBus bus;
  XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
  XWalkPwmTimerState timerState;
  XWalkPwm firstPwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
  XWalkPwm secondPwm(i2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
  XWalkServo firstServo(firstPwm);
  XWalkServo secondServo(secondPwm);
  XWalkConfigStore store(filePath.string());
  store.set("walker_servo_offset_list", "[5,-5]");
  XWalkRobot robot(store, "walker", 0U);
  robot.addServo(firstServo, 10.0);
  robot.addServo(secondServo, -10.0);
  robot.initialize({1U, 0U});
  assert(robot.initialized());
  assert(robot.servoCount() == 2U);
  assert(robot.offsets() == float64vector({5.0, -5.0}));
  assert(robot.servoPositions() == float64vector({10.0, -10.0}));
  robot.setOriginPositions({1.0, 2.0});
  robot.setDirections({1.0, -1.0});
  robot.setCalibrationPositions({3.0, 4.0});
  robot.calibration();
  assert(robot.servoPositions() == float64vector({3.0, 4.0}));
  robot.setOffsets({30.0, -30.0});
  assert(robot.offsets() == float64vector({20.0, -20.0}));
  assert(store.get("walker_servo_offset_list") == "[20.000000,-20.000000]");
  robot.reset({0.0, 0.0});
  robot.servoMove({2.0, -2.0}, 100.0);
  assert(robot.servoPositions() == float64vector({2.0, -2.0}));
  robot.setAction("wave", {{1.0, -1.0}, {2.0, -2.0}});
  robot.doAction("wave", 1U, 100.0);
  assert(robot.servoPositions() == float64vector({2.0, -2.0}));
  const float64vector beforeSoftReset = robot.servoPositions();
  robot.softReset();
  assert(robot.servoPositions() == beforeSoftReset);
  robot.reset();
  assert(robot.servoPositions() == float64vector({0.0, 0.0}));
  assert(bus.writeCount > 0U);
}
void testValidation(const filesystempath &filePath) {
  TestBus bus;
  XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
  XWalkPwmTimerState timerState;
  XWalkPwm pwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
  XWalkServo servo(pwm);
  XWalkConfigStore store(filePath.string());
  XWalkRobot robot(store, "validation", 0U);
  xwalk::hal::test::expectFailure([&]() { robot.initialize(); });
  robot.addServo(servo);
  robot.initialize();
  xwalk::hal::test::expectFailure([&]() { robot.reset({0.0, 1.0}); });
  xwalk::hal::test::expectFailure([&]() { robot.doAction("missing"); });
}
/** @brief Verifies Robot simulation trace argument boundaries. */
void testSimulationArguments() {
  char binary[] = "xWalkRobotSimulation";
  char help[] = "--help";
  char shortHelp[] = "-h";
  char trace[] = "--trace";
  char enable[] = "RPI.enable";
  char disable[] = "all.disable";
  char json[] = "trace.json";
  char malformed[] = "RPI.Camera.enable";
  char unknown[] = "--verbose";
  charpointer defaults[]{binary};
  charpointer helpValues[]{binary, help};
  charpointer shortHelpValues[]{binary, shortHelp};
  charpointer enableValues[]{binary, trace, enable};
  charpointer disableValues[]{binary, trace, disable};
  charpointer jsonValues[]{binary, trace, json};
  charpointer malformedValues[]{binary, trace, malformed};
  charpointer unknownValues[]{binary, unknown, enable};
  const sim::XWalkRobotSimulationArguments defaultArguments(1, defaults);
  const sim::XWalkRobotSimulationArguments helpArguments(2, helpValues);
  const sim::XWalkRobotSimulationArguments shortHelpArguments(2, shortHelpValues);
  const sim::XWalkRobotSimulationArguments enableArguments(3, enableValues);
  const sim::XWalkRobotSimulationArguments disableArguments(3, disableValues);
  const sim::XWalkRobotSimulationArguments jsonArguments(3, jsonValues);
  const sim::XWalkRobotSimulationArguments malformedArguments(3, malformedValues);
  const sim::XWalkRobotSimulationArguments unknownArguments(3, unknownValues);
  const sim::XWalkRobotSimulationArguments nullArguments(3, nullptr);
  assert(defaultArguments.valid() && defaultArguments.applyTraceUpdate());
  assert(helpArguments.valid() && helpArguments.helpRequested());
  assert(shortHelpArguments.valid() && shortHelpArguments.helpRequested());
  assert(enableArguments.valid() && enableArguments.applyTraceUpdate());
  assert(disableArguments.valid() && disableArguments.applyTraceUpdate());
  assert(jsonArguments.valid());
  assert(!malformedArguments.valid());
  assert(!unknownArguments.valid());
  assert(!nullArguments.valid());
}
} /* namespace */
XWalkHal::int32 main(XWalkHal::int32 count, XWalkHal::charpointer values[]) {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_ROBOT_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_ROBOT_SIMULATION_TRACE_LOG_PATH);
  if (count != 2) {
    return 1;
  }
  const XWalkHal::filesystempath path(values[1]);
  XWalkHal::filesystempath temporary = path;
  temporary += ".tmp";
  static_cast<void>(XWalkHal::removeFilesystemEntry(path));
  static_cast<void>(XWalkHal::removeFilesystemEntry(temporary));
  XWALK_HAL_TRACE_UID0(RPI .355, "xWalkRobot host tests started");
  testRobot(path);
  static_cast<void>(XWalkHal::removeFilesystemEntry(path));
  testValidation(path);
  testSimulationArguments();
  XWALK_HAL_TRACE_UID0(RPI .356, "xWalkRobot host tests completed");
  static_cast<void>(XWalkHal::removeFilesystemEntry(path));
  static_cast<void>(XWalkHal::removeFilesystemEntry(temporary));
  static_cast<void>(XWalkHal::removeFilesystemEntry(path.parent_path()));
  return 0;
}
