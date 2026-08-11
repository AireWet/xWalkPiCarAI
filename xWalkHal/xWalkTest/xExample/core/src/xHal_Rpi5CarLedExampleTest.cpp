/******************************************************************************
 * @file        xHal_Rpi5CarLedExampleTest.cpp
 * @brief       Verifies the ported LED example without physical GPIO access.
 *
 * @details
 * Checks status messages, action order, waits, blink parameters, callback
 * validation, and exceptional close behavior through an in-memory adapter.
 *
 * @project     xWalk Firmware
 * @module      xExample Host Test
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

#include "xHal_Rpi5CarLedExample.h"

#include "xHal_Rpi5CarTrace.h"
#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory adapter and host verification scenarios. */
namespace {

/** @brief Records every observable LED example operation. */
struct LedExampleState {
  /** @brief Ordered LED operation names. */
  XWalkHal::stringvector operations;
  /** @brief Ordered source-compatible status messages. */
  XWalkHal::stringvector messages;
  /** @brief Ordered wait durations in milliseconds. */
  XWalkHal::uint32vector waits;
  /** @brief Ordered blink cycle counts. */
  XWalkHal::uint32vector blinkCounts;
  /** @brief Ordered blink transition delays in seconds. */
  XWalkHal::float64vector blinkDelays;
  /** @brief Ordered blink pauses in seconds. */
  XWalkHal::float64vector blinkPauses;
  /** @brief One-based wait callback that must throw, or zero for none. */
  XWalkHal::uint32 failingWait{};
};

/** @brief Records one LED activation. */
void on(XWalkHal::contextpointer context) {
  static_cast<LedExampleState *>(context)->operations.push_back("on");
}

/** @brief Records one LED deactivation. */
void off(XWalkHal::contextpointer context) {
  static_cast<LedExampleState *>(context)->operations.push_back("off");
}

/** @brief Records one complete blink configuration. */
void blink(XWalkHal::contextpointer context, XWalkHal::uint32 cycleCount,
           XWalkHal::float64 toggleDelaySeconds,
           XWalkHal::float64 pauseSeconds) {
  LedExampleState &state = *static_cast<LedExampleState *>(context);
  state.operations.push_back("blink");
  state.blinkCounts.push_back(cycleCount);
  state.blinkDelays.push_back(toggleDelaySeconds);
  state.blinkPauses.push_back(pauseSeconds);
}

/** @brief Records one LED close operation. */
void close(XWalkHal::contextpointer context) {
  static_cast<LedExampleState *>(context)->operations.push_back("close");
}

/** @brief Records one wait or raises the configured simulated failure. */
void wait(XWalkHal::contextpointer context,
          XWalkHal::uint32 durationMilliseconds) {
  LedExampleState &state = *static_cast<LedExampleState *>(context);
  state.waits.push_back(durationMilliseconds);
  const hal::boolean xWalkHalStateFailingWaitMatched =
      static_cast<hal::boolean>(
          static_cast<XWalkHal::size>(state.failingWait) == state.waits.size());
  if (xWalkHalStateFailingWaitMatched) {
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Simulated LED example wait failure");
  }
}

/** @brief Records one source-compatible status message. */
void report(XWalkHal::contextpointer context, XWalkHal::stringview message) {
  static_cast<LedExampleState *>(context)->messages.emplace_back(message);
}

/** @brief Returns the complete in-memory callback table. */
xwalk::hal::example::XWalkLedExampleCallbacks callbacks() {
  return {&on, &off, &blink, &close, &wait, &report};
}

/** @brief Verifies the complete normal example flow. */
void testNormalFlow() {
  LedExampleState state;
  xwalk::hal::example::XWalkLedExample example(&state, callbacks());

  example.run();

  const XWalkHal::stringvector expectedOperations{"on",    "off",   "blink",
                                                  "blink", "blink", "close"};
  const XWalkHal::stringvector expectedMessages{
      "On",
      "Off",
      "Blink delay 1 second",
      "Blink 3 times delay 0.1 second pause 0.5 second",
      "Blink 2 times delay 0.2 second pause 1 second",
      "Done"};
  const XWalkHal::uint32vector expectedWaits{2'000U, 2'000U, 5'000U, 5'000U,
                                             5'000U};
  const XWalkHal::uint32vector expectedBlinkCounts{1U, 3U, 2U};
  const XWalkHal::float64vector expectedBlinkDelays{1.0, 0.1, 0.2};
  const XWalkHal::float64vector expectedBlinkPauses{0.0, 0.5, 1.0};
  assert(state.operations == expectedOperations);
  assert(state.messages == expectedMessages);
  assert(state.waits == expectedWaits);
  assert(state.blinkCounts == expectedBlinkCounts);
  assert(state.blinkDelays == expectedBlinkDelays);
  assert(state.blinkPauses == expectedBlinkPauses);
}

/** @brief Verifies callback validation and best-effort exceptional close. */
void testValidationAndCleanup() {
  LedExampleState state;
  xwalk::hal::example::XWalkLedExampleCallbacks incompleteCallbacks =
      callbacks();
  incompleteCallbacks.blink = nullptr;
  XWalkHal::boolean rejectedCallbacks = false;
  try {
    xwalk::hal::example::XWalkLedExample invalidExample(&state,
                                                        incompleteCallbacks);
  } catch (const XWalkHal::invalidargument &) {
    rejectedCallbacks = true;
  }
  assert(rejectedCallbacks);

  state.failingWait = 1U;
  xwalk::hal::example::XWalkLedExample failingExample(&state, callbacks());
  XWalkHal::boolean propagatedFailure = false;
  try {
    failingExample.run();
  } catch (const XWalkHal::runtimeerror &) {
    propagatedFailure = true;
  }
  assert(propagatedFailure);
  assert(state.operations == XWalkHal::stringvector({"on", "close"}));
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe LED example verification.
 *
 * @return Zero after every assertion passes.
 */
int xWalkLedExampleHostTest() {
  testNormalFlow();
  testValidationAndCleanup();
  return 0;
}
