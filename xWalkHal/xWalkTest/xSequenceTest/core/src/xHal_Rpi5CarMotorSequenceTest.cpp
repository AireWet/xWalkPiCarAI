/******************************************************************************
 * @file        xHal_Rpi5CarMotorSequenceTest.cpp
 * @brief       Verifies the two-motor Robot HAT sequence in memory.
 *
 * @details
 * Checks the P13/D4 and P12/D5 mapping, signed-speed phases, direction output,
 * bounded waits, final stop behavior, and validation without moving hardware.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Host Test
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarMotorSequence.h"

#include "xHal_Rpi5CarTrace.h"
#include <cassert>
#include <vector>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace {

struct TestI2c {
  XWalkHal::uint32 writeCount{};
};

struct TestGpio {
  XWalkHal::uint8 pin{};
  XWalkHal::boolean value{};
};

struct WaitState {
  XWalkHal::fixedarray<XWalkHal::XWalkMotor *, 2U> motors{};
  XWalkHal::fixedarray<TestGpio *, 2U> directions{};
  XWalkHal::uint32vector durations;
  XWalkHal::float64vector observedSpeeds;
  std::vector<XWalkHal::boolean> observedDirections;
  XWalkHal::uint32 failureWait{};
};

XWalkHal::boolean probe(XWalkHal::contextpointer context,
                        XWalkHal::uint8 address) {
  static_cast<void>(context);
  static_cast<void>(address);
  return true;
}

void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
                   XWalkHal::uint8 reg, const XWalkHal::bytevector &data) {
  TestI2c &state = *static_cast<TestI2c *>(context);
  static_cast<void>(address);
  static_cast<void>(reg);
  static_cast<void>(data);
  ++state.writeCount;
}

XWalkHal::boolean tryWriteRegister(XWalkHal::contextpointer context,
                                   XWalkHal::uint8 address, XWalkHal::uint8 reg,
                                   const XWalkHal::bytevector &data) noexcept {
  TestI2c &state = *static_cast<TestI2c *>(context);
  static_cast<void>(address);
  static_cast<void>(reg);
  static_cast<void>(data);
  ++state.writeCount;
  return true;
}

XWalkHal::bytevector read(XWalkHal::contextpointer context,
                          XWalkHal::uint8 address, XWalkHal::size length) {
  static_cast<void>(context);
  static_cast<void>(address);
  return XWalkHal::bytevector(length, 0U);
}

void configureGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
                   XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull,
                   XWalkHal::boolean initialValue) {
  TestGpio &state = *static_cast<TestGpio *>(context);
  state.pin = pin;
  state.value = initialValue;
  assert(mode == XWalkHal::XWalkGpioMode::Output);
  assert(pull == XWalkHal::XWalkGpioPull::None);
}

XWalkHal::boolean readGpio(XWalkHal::contextpointer context,
                           XWalkHal::uint8 pin) {
  const TestGpio &state = *static_cast<TestGpio *>(context);
  assert(pin == state.pin);
  return state.value;
}

void writeGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
               XWalkHal::boolean value) {
  TestGpio &state = *static_cast<TestGpio *>(context);
  assert(pin == state.pin);
  state.value = value;
}

void interruptGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
                   XWalkHal::XWalkGpioEdge edge,
                   XWalkHal::uint32 debounceMilliseconds,
                   XWalkHal::contextpointer handlerContext,
                   XWalkHal::gpiointerrupthandler handler) {
  static_cast<void>(context);
  static_cast<void>(pin);
  static_cast<void>(edge);
  static_cast<void>(debounceMilliseconds);
  static_cast<void>(handlerContext);
  static_cast<void>(handler);
}

void cancelInterruptGpio(XWalkHal::contextpointer context,
                         XWalkHal::uint8 pin) {
  static_cast<void>(context);
  static_cast<void>(pin);
}

XWalkHal::XWalkGpioCallbacks gpioCallbacks() {
  return {&configureGpio, &readGpio, &writeGpio, &interruptGpio,
          &cancelInterruptGpio};
}

void wait(XWalkHal::contextpointer context,
          XWalkHal::uint32 durationMilliseconds) {
  WaitState &state = *static_cast<WaitState *>(context);
  state.durations.push_back(durationMilliseconds);
  for (XWalkHal::size index = 0U; index < state.motors.size(); ++index) {
    state.observedSpeeds.push_back(state.motors[index]->speed());
    state.observedDirections.push_back(state.directions[index]->value);
  }
  const hal::boolean failureDelayObserved =
      static_cast<hal::boolean>((state.failureWait != 0U) &&
                                (state.durations.size() == state.failureWait));
  if (failureDelayObserved) {
    state.failureWait = 0U;
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Simulated motor-sequence wait failure");
  }
}

void runTest() {
  TestI2c i2cState;
  XWalkHal::XWalkI2c i2c(&i2cState, &probe, &writeRegister, &read, nullptr,
                         &tryWriteRegister);
  XWalkHal::XWalkPwmTimerState timerState;
  XWalkHal::XWalkPwm pwm13(i2c, 13U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
  XWalkHal::XWalkPwm pwm12(i2c, 12U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
  TestGpio directionD4;
  TestGpio directionD5;
  const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
  XWalkHal::XWalkGpio gpioD4(&directionD4, callbacks, "D4");
  XWalkHal::XWalkGpio gpioD5(&directionD5, callbacks, "D5");
  XWalkHal::XWalkMotor firstMotor(pwm13, gpioD4);
  XWalkHal::XWalkMotor secondMotor(pwm12, gpioD5);
  WaitState waitState;
  waitState.motors = {{&firstMotor, &secondMotor}};
  waitState.directions = {{&directionD4, &directionD5}};
  xwalk::hal::test::XWalkMotorSequence sequence(firstMotor, secondMotor,
                                                &waitState, &wait);

  sequence.run(1U);

  assert(directionD4.pin == 23U);
  assert(directionD5.pin == 24U);
  assert(waitState.durations == XWalkHal::uint32vector({1'000U, 1'000U, 100U}));
  assert(waitState.observedSpeeds ==
         XWalkHal::float64vector({-50.0, -50.0, 50.0, 50.0, 0.0, 0.0}));
  assert(
      waitState.observedDirections ==
      std::vector<XWalkHal::boolean>({false, false, true, true, false, false}));
  assert(firstMotor.speed() == 0.0);
  assert(secondMotor.speed() == 0.0);
  assert(i2cState.writeCount > 0U);

  XWalkHal::boolean rejectedCycles = false;
  try {
    sequence.run(0U);
  } catch (const std::out_of_range &) {
    rejectedCycles = true;
  }
  assert(rejectedCycles);

  waitState.durations.clear();
  waitState.observedSpeeds.clear();
  waitState.observedDirections.clear();
  waitState.failureWait = 2U;
  XWalkHal::boolean propagatedFailure = false;
  try {
    sequence.run(1U);
  } catch (const std::runtime_error &) {
    propagatedFailure = true;
  }
  assert(propagatedFailure);
  assert(waitState.durations == XWalkHal::uint32vector({1'000U, 1'000U, 100U}));
  assert(firstMotor.speed() == 0.0);
  assert(secondMotor.speed() == 0.0);
  assert(!directionD4.value);
  assert(!directionD5.value);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/** @brief Runs the host-safe two-motor sequence verification. */
int xWalkMotorSequenceHostTest() {
  runTest();
  return 0;
}
