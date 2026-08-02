/******************************************************************************
 * @file        xAgent_Rpi5CarBootTestStub.h
 * @brief       Declares host-only fixtures for xWalkBoot stub verification.
 *
 * @details
 * Provides the invocation state and application callback used by the
 * device-free xWalkBoot host-stub test.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot Host Stub Test
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_BOOT_TEST_STUB_H
#define XAGENT_RPI5CAR_BOOT_TEST_STUB_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBootTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent::test
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Counts host-stub application invocations. */
struct XWalkBootTestState
{
    /** @brief Number of application callback invocations. */
    hal::uint32 runCount{};
};

/******************************************************************************
 * Function declarations
 ******************************************************************************/

/**
 * @brief Verifies that the exact simulated service table is forwarded.
 * @param[in,out] context Non-null pointer to an `XWalkBootTestState` object.
 * @param[in] services Simulated services supplied by the host boot stub.
 * @return Deterministic test status value seven.
 */
hal::int32 runBootTestApplication(hal::contextpointer context,
    XWalkBootServices& services);

} /* namespace xwalk::agent::test */

#endif /* XAGENT_RPI5CAR_BOOT_TEST_STUB_H */
