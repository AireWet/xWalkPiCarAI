/******************************************************************************
 * @file        xHal_Rpi5CarUtilsHostStub.cpp
 * @brief       Implements the side-effect-free xWalkUtils host stub.
 * @details     Records utility callbacks in memory and returns deterministic
 *query values.
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#include "xHal_Rpi5CarUtilsHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
XWalkUtilsHostStub::XWalkUtilsHostStub() = default;
XWalkUtilsHostStub::~XWalkUtilsHostStub() = default;
void XWalkUtilsHostStub::output(contextpointer context, XWalkUtilityColor color,
                                stringview message, stringview ending,
                                boolean flush) {
  static_cast<void>(color);
  static_cast<void>(ending);
  static_cast<void>(flush);
  auto &stub = *static_cast<XWalkUtilsHostStub *>(context);
  stub.messageValue = string(message);
  ++stub.outputCountValue;
  const string ownedMessage(message);
  XWALK_HAL_TRACE_UID1(RPI .133, "Host Utils mirrored output: %s",
                       ownedMessage.c_str());
}
void XWalkUtilsHostStub::setVolume(contextpointer context,
                                   uint8 volumePercent) {
  static_cast<XWalkUtilsHostStub *>(context)->volumePercentValue =
      volumePercent;
  XWALK_HAL_TRACE_UID1(RPI .134, "Host Utils mirrored volume %u percent",
                       static_cast<uint32>(volumePercent));
}
XWalkCommandResult XWalkUtilsHostStub::runCommand(contextpointer context,
                                                  stringview command,
                                                  stringview user,
                                                  stringview group) {
  static_cast<void>(user);
  static_cast<void>(group);
  auto &stub = *static_cast<XWalkUtilsHostStub *>(context);
  stub.commandValue = string(command);
  const string ownedCommand(command);
  XWALK_HAL_TRACE_UID1(RPI .135, "Host Utils mirrored command: %s",
                       ownedCommand.c_str());
  return {0, "mirrored command output"};
}
boolean XWalkUtilsHostStub::executableExists(contextpointer context,
                                             stringview executable) {
  static_cast<void>(context);
  return executable == "xwalk-tool";
}
string XWalkUtilsHostStub::ipAddress(contextpointer context,
                                     stringview interfaceName) {
  static_cast<void>(context);
  return (interfaceName == "eth0") ? string("192.0.2.10") : string{};
}
string XWalkUtilsHostStub::username(contextpointer context) {
  static_cast<void>(context);
  return "xwalk";
}
XWalkUtilsCallbacks XWalkUtilsHostStub::callbacks() noexcept {
  return {&output,           &setVolume, &runCommand,
          &executableExists, &ipAddress, &username};
}
string XWalkUtilsHostStub::message() const { return messageValue; }
string XWalkUtilsHostStub::command() const { return commandValue; }
uint8 XWalkUtilsHostStub::volumePercent() const noexcept {
  return volumePercentValue;
}
uint32 XWalkUtilsHostStub::outputCount() const noexcept {
  return outputCountValue;
}
} /* namespace xwalk::hal::sim */
