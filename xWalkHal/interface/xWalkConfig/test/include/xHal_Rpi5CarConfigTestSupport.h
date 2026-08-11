/******************************************************************************
 * @file        xHal_Rpi5CarConfigTestSupport.h
 * @brief       Declares reusable xWalkConfig host-test support.
 *
 * @details
 * Provides trace-backed expectations and deterministic section-file fixtures
 * outside scenario translation units.
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

#ifndef XHAL_RPI5CAR_CONFIG_TEST_SUPPORT_H
#define XHAL_RPI5CAR_CONFIG_TEST_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::test::config
{

/**
 * @brief Reports a failed expectation through the trace service.
 * @param[in] condition Result that must be true for success.
 * @param[in] message Diagnostic retained only for synchronous trace output.
 * @return The unchanged `condition` value.
 */
boolean expect(boolean condition, stringview message);

/**
 * @brief Writes deterministic initial section-aware configuration content.
 * @param[in] path Test-owned file path whose parent directory exists.
 */
void writeFixture(const filesystempath& path);

} /* namespace xwalk::hal::test::config */

#endif /* XHAL_RPI5CAR_CONFIG_TEST_SUPPORT_H */
