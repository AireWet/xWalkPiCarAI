/******************************************************************************
 * @file        xHal_Rpi5CarConfigTestSupport.cpp
 * @brief       Implements reusable xWalkConfig host-test support.
 *
 * @details
 * Emits expectation diagnostics through trace macros and writes a deterministic
 * test-owned fixture without exposing helpers from scenario source files.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig Host Test
 *
 * @author      Joxy John
 * @date        2026-08-10
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

#include "xHal_Rpi5CarConfigTestSupport.h"

#include "xHal_Rpi5CarTrace.h"

#include <fstream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test::config {

/**
 * @brief Reports a failed expectation through the trace service.
 * @param[in] condition Result that must be true for success.
 * @param[in] message Diagnostic retained only for synchronous trace output.
 * @return The unchanged `condition` value.
 */
boolean expect(boolean condition, stringview message) {
  if (condition == false) {
    const string ownedMessage(message);
    XWALK_HAL_ERROR(XWALK_EXCEPTION, "xWalkConfig test expectation failed: %s",
                    ownedMessage.c_str());
  }
  return condition;
}

/**
 * @brief Writes deterministic initial section-aware configuration content.
 * @param[in] path Test-owned file path whose parent directory exists.
 */
void writeFixture(const filesystempath &path) {
  outputfilestream file(path, FILE_OPEN_WRITE_TRUNCATE);
  file << "# retained comment\n";
  file << "root = first\n\n";
  file << "[motor]\n";
  file << "speed = 40\n";
  file << "direction = forward\n";
}

} /* namespace xwalk::hal::test::config */
